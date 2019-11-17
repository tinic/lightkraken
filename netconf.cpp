/*
Copyright 2019 Tinic Uro

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <stdio.h>
#include <stdint.h>

extern "C" {
#include "gd32f10x.h"
#include "cmsis_gcc.h"

#include "lwip/init.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/apps/httpd.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/inet_chksum.h"
}; //extern "C" {

#include "./main.h"
#include "./ethernetif.h"
#include "./netconf.h"
#include "./artnet.h"
#include "./sacn.h"
#include "./model.h"
#include "./artnet.h"
#include "./systick.h"
#include "./perf.h"

namespace lightkraken {

NetConf &NetConf::instance() {
    static NetConf netconf;
    if (!netconf.initialized) {
        netconf.initialized = true;
        netconf.init();
    }
    return netconf;
}

#ifndef BOOTLOADER
static struct udp_pcb *upcb_in_artnet = 0;

static void udp_receive_artnet_callback(void *, struct udp_pcb *, struct pbuf *p, const ip_addr_t *from, u16_t) {
    struct pbuf *i = p;
    for( ; i != NULL ; i = i->next) {
        bool isBroadcast = ip4_addr_isbroadcast(from, NetConf::instance().netInterface());
        lightkraken::ArtNetPacket::dispatch(from, reinterpret_cast<uint8_t *>(p->payload), p->len, isBroadcast);
    }
    pbuf_free(p);
}

static struct udp_pcb *upcb_in_sacn = 0;

static void udp_receive_sacn_callback(void *, struct udp_pcb *, struct pbuf *p, const ip_addr_t *from, u16_t) {
    struct pbuf *i = p;
    for( ; i != NULL ; i = i->next) {
        bool isBroadcast = ip4_addr_isbroadcast(from, NetConf::instance().netInterface());
        lightkraken::sACNPacket::dispatch(from, reinterpret_cast<uint8_t *>(p->payload), p->len, isBroadcast);
    }
    pbuf_free(p);
}
#endif  // #ifndef BOOTLOADER

void NetConf::init() {
    lwip_init();
    ip_addr_t address;
    ip_addr_t netmask;
    ip_addr_t gateway;

#ifndef BOOTLOADER
#if LWIP_DHCP
    if (lightkraken::Model::instance().dhcpEnabled()) {
    address.addr = 0;
    netmask.addr = 0;
    gateway.addr = 0;
    } else 
#endif  // #if LWIP_DHCP
    {
    address.addr = lightkraken::Model::instance().ip4Address()->addr;
    netmask.addr = lightkraken::Model::instance().ip4Netmask()->addr;
    gateway.addr = lightkraken::Model::instance().ip4Gateway()->addr;
    }
#else  // #ifndef BOOTLOADER
    address.addr = 0;
    netmask.addr = 0;
    gateway.addr = 0;
    multicast.addr = 0;
#endif  // #ifndef BOOTLOADER

    netif_add(&netif, &address, &netmask, &gateway, NULL, &EthernetIf::ethernetif_init, &ethernet_input);

    netif_set_default(&netif);

    if (netif_is_link_up(&netif)) {
        DEBUG_PRINTF(("ENET link is up.\n"));
        netif_set_up(&netif);
    } else {
        netif_set_down(&netif);
        DEBUG_PRINTF(("ENET link is down.\n"));
    }

#ifndef BOOTLOADER
    upcb_in_artnet = udp_new();
    ip_set_option(upcb_in_artnet, SOF_BROADCAST);
    if (udp_bind(upcb_in_artnet, IP4_ADDR_ANY, ArtNetPacket::port) == ERR_OK) {
        udp_recv(upcb_in_artnet, udp_receive_artnet_callback, NULL);
    } else {
        udp_remove(upcb_in_artnet);
        upcb_in_artnet = 0;
    }

    upcb_in_sacn = udp_new();
    ip_set_option(upcb_in_sacn, SOF_BROADCAST);
    if (udp_bind(upcb_in_sacn, IP4_ADDR_ANY, sACNPacket::ACN_SDT_MULTICAST_PORT) == ERR_OK) {
        udp_recv(upcb_in_sacn, udp_receive_sacn_callback, NULL);
    } else {
        udp_remove(upcb_in_sacn);
        upcb_in_sacn = 0;
    }

	if (!lightkraken::Model::instance().dhcpEnabled()) {
		sACNPacket::joinNetworks();
	}
#endif  // #ifndef BOOTLOADER
    
    httpd_init();	
}

#ifndef BOOTLOADER
bool NetConf::sendArtNetUdpPacket(const ip_addr_t *to, const uint16_t port, const uint8_t *data, uint16_t len) {
    
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	if (p == NULL) {
		return false;
	}
	
	err_t err = pbuf_take(p, data, len);
	if (err != ERR_OK) {
		return false;
	}
	
	udp_connect(upcb_in_artnet, to, port);
	udp_send(upcb_in_artnet, p);
    udp_disconnect(upcb_in_artnet);
    
    pbuf_free(p);

	return err == ERR_OK;
}

bool NetConf::sendsACNUdpPacket(const ip_addr_t *to, const uint16_t port, const uint8_t *data, uint16_t len) {
    
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	if (p == NULL) {
		return false;
	}
	
	err_t err = pbuf_take(p, data, len);
	if (err != ERR_OK) {
		return false;
	}
	
	udp_connect(upcb_in_sacn, to, port);
	udp_send(upcb_in_sacn, p);
    udp_disconnect(upcb_in_sacn);
    
    pbuf_free(p);

	return err == ERR_OK;
}
#endif  // #ifndef BOOTLOADER

void NetConf::update() {

    uint32_t localtime = lightkraken::Systick::instance().systemTime();

    if (enet_rxframe_size_get()){
		PerfMeasure perf(PerfMeasure::SLOT_ENET_INPUT);
        EthernetIf::ethernetif_input(&netif);
    }

    if (localtime - tcp_timer >= TCP_TMR_INTERVAL){
        tcp_timer =  localtime;
        tcp_tmr();
    }

    if ((localtime - arp_timer) >= ARP_TMR_INTERVAL){ 
        arp_timer =  localtime;
        etharp_tmr();
    }

#ifndef BOOTLOADER
    if ((localtime - config_timer) >= 1000){ 
        config_timer =  localtime;
    }
#endif  // #ifndef BOOTLOADER

#if LWIP_DHCP
    const int32_t MAX_DHCP_TRIES = 4;

    if (localtime - dhcp_fine_timer >= DHCP_FINE_TIMER_MSECS){
        dhcp_fine_timer =  localtime;
        dhcp_fine_tmr();
        if ((dhcp_state != DHCP_ADDRESS_ASSIGNED) && (dhcp_state != DHCP_TIMEOUT)){ 
            struct dhcp *dhcp_client = netif_dhcp_data(&netif);
            switch (dhcp_state){
            case DHCP_START:
                DEBUG_PRINTF(("DHCP start...\n"));
                dhcp_start(&netif);
                dhcp_state = DHCP_WAIT_ADDRESS;
                break;
            case DHCP_WAIT_ADDRESS:
                ip_addr_t address;
                ip_address.addr = netif.ip_addr.addr;
                if (ip_address.addr != 0){ 
                    DEBUG_PRINTF(("DHCP address: %d.%d.%d.%d\n", ip4_addr1(&ip_address), ip4_addr2(&ip_address), ip4_addr3(&ip_address),ip4_addr4(&ip_address)));
                    dhcp_state = DHCP_ADDRESS_ASSIGNED;
	                sACNPacket::joinNetworks();
                } else {
                    if (dhcp_client->tries > MAX_DHCP_TRIES){
                        dhcp_state = DHCP_TIMEOUT;
                        dhcp_stop(&netif);
                        DEBUG_PRINTF(("DHCP timeout.\n"));

                        ip_addr_t netmask;
                        ip_addr_t gateway;

#ifndef BOOTLOADER
                        address.addr = lightkraken::Model::instance().ip4Address()->addr;
                        netmask.addr = lightkraken::Model::instance().ip4Netmask()->addr;
                        gateway.addr = lightkraken::Model::instance().ip4Gateway()->addr;
#endif  // #ifndef BOOTLOADER

                        netif_set_addr(&netif, &address , &netmask, &gateway);
                    }
                }
                break;
            default: 
                break;
            }
        }
    }

    if (localtime - dhcp_coarse_timer >= DHCP_COARSE_TIMER_MSECS){
        dhcp_coarse_timer =  localtime;
        dhcp_coarse_tmr();
    }
#endif  // #if LWIP_DHCP
}

}


