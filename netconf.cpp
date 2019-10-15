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
}; //extern "C" {

#include "hardware.h"
#include "ethernetif.h"
#include "netconf.h"

#define MAX_DHCP_TRIES        4

typedef enum 
{ 
    DHCP_START=0,
    DHCP_WAIT_ADDRESS,
    DHCP_ADDRESS_ASSIGNED,
    DHCP_TIMEOUT
} dhcp_state_enum;

uint32_t dhcp_fine_timer = 0;
uint32_t dhcp_coarse_timer = 0;
dhcp_state_enum dhcp_state = DHCP_START;

struct netif netif;
uint32_t tcp_timer = 0;
uint32_t arp_timer = 0;
ip_addr_t ip_address = {0};

u32_t sys_now(void) {
    return system_time();
}

void lwip_pkt_handle(void) {
    ethernetif_input(&netif);
}

void lwip_periodic_handle(__IO uint32_t localtime) {
    if (localtime - tcp_timer >= TCP_TMR_INTERVAL){
        tcp_timer =  localtime;
        tcp_tmr();
    }
  
    if ((localtime - arp_timer) >= ARP_TMR_INTERVAL){ 
        arp_timer =  localtime;
        etharp_tmr();
    }

    if (localtime - dhcp_fine_timer >= DHCP_FINE_TIMER_MSECS){
        dhcp_fine_timer =  localtime;
        dhcp_fine_tmr();
        if ((dhcp_state != DHCP_ADDRESS_ASSIGNED) && (dhcp_state != DHCP_TIMEOUT)){ 
            lwip_dhcp_process_handle();    
        }
    }

    if (localtime - dhcp_coarse_timer >= DHCP_COARSE_TIMER_MSECS){
        dhcp_coarse_timer =  localtime;
        dhcp_coarse_tmr();
    }
}

void lwip_dhcp_process_handle(void) {
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;
    struct dhcp *dhcp_client;
    
    dhcp_client = netif_dhcp_data(&netif);
    
    switch (dhcp_state){
    case DHCP_START:
        dhcp_start(&netif);
        ip_address.addr = 0;
        dhcp_state = DHCP_WAIT_ADDRESS;
        break;

    case DHCP_WAIT_ADDRESS:
        ip_address.addr = netif.ip_addr.addr;

        if (ip_address.addr != 0){ 
            dhcp_state = DHCP_ADDRESS_ASSIGNED;
            dhcp_stop(&netif);
        }else{
            if (dhcp_client->tries > MAX_DHCP_TRIES){
                dhcp_state = DHCP_TIMEOUT;
                dhcp_stop(&netif);

                IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
                IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
                IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
                netif_set_addr(&netif, &ipaddr , &netmask, &gw);
            }
        }
        break;

    default: 
        break;
    }
}

void lwip_stack_init(void) {
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    lwip_init();

    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;

    netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

    netif_set_default(&netif);

    if (netif_is_link_up(&netif)) {
        netif_set_up(&netif);
    } else {
        netif_set_down(&netif);
    }

    dhcp_start(&netif);

	httpd_init();
}

