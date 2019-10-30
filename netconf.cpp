/*!
    \file    netconf.c
    \brief   network connection configuration 

    \version 2014-12-26, V1.0.0, firmware for GD32F10x
    \version 2017-06-20, V2.0.0, firmware for GD32F10x
    \version 2018-07-31, V2.1.0, firmware for GD32F10x
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
    list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation 
    and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
    may be used to endorse or promote products derived from this software without 
    specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
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
#include "./model.h"
#include "./artnet.h"
#include "./systick.h"

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
static void udp_receive_artnet_callback(void *, struct udp_pcb *, struct pbuf *p, const ip_addr_t *from, u16_t) {
    struct pbuf *i = p;
    for( ; i != NULL ; i = i->next) {
        lightkraken::ArtNetPacket::dispatch(from, reinterpret_cast<uint8_t *>(p->payload), p->len);
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
    static struct udp_pcb *upcb_artnet = 0;
    upcb_artnet = udp_new();
    if (udp_bind(upcb_artnet, IP4_ADDR_ANY, 6454) == ERR_OK) {
        udp_recv(upcb_artnet, udp_receive_artnet_callback, NULL);
    } else {
        udp_remove(upcb_artnet);
        upcb_artnet = 0;
    }
#endif  // #ifndef BOOTLOADER
    
    httpd_init();
}

bool NetConf::sendUdpPacket(const ip_addr_t *to, const uint16_t port, const uint8_t *data, uint16_t len) {

	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	if (p == NULL) {
		return false;
	}
	
	err_t err = pbuf_take(p, data, len);
	if (err != ERR_OK) {
		return false;
	}

	/* add UDP header */
	pbuf_add_header(p, sizeof(struct udp_hdr));
	struct udp_hdr *uh = (struct udp_hdr *)p->payload;
	uh->chksum = 0;
	uh->dest = uh->src = lwip_htons(port);
	uh->len = lwip_htons(p->tot_len);

	/* add IPv4 header */
	pbuf_add_header(p, sizeof(struct ip_hdr));
	struct ip_hdr *ih = (struct ip_hdr *)p->payload;
	memset(ih, 0, sizeof(*ih));
	ih->dest.addr = to->addr;
	ih->_len = lwip_htons(p->tot_len);
	ih->_ttl = 32;
	ih->_proto = IP_PROTO_UDP;
	IPH_VHL_SET(ih, 4, sizeof(struct ip_hdr) / 4);
	IPH_CHKSUM_SET(ih, inet_chksum(ih, sizeof(struct ip_hdr)));

	err = ip4_input(p, &netif);

	return err == ERR_OK;
}

void NetConf::update() {

    uint32_t localtime = lightkraken::Systick::instance().systemTime();

    if (enet_rxframe_size_get()){
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


