/*!
    \file    lwipopts.h
    \brief   LwIP options configuration 

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

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

#include <stdio.h>

#define HTTPC_CLIENT_AGENT      "LG/1.0"
#define HTTPD_SERVER_AGENT      "LG/1.0"

#define LWIP_TCPIP_CORE_LOCKING 0						 /* no threading */

#define SYS_LIGHTWEIGHT_PROT    0                        /* SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection 
                                                            for certain critical regions during buffer allocation,
                                                            deallocation and memory allocation and deallocation */                                                            

#define NO_SYS                  1                        /* NO_SYS==1: provides VERY minimal functionality. 
                                                            Otherwise, use lwIP facilities */

/*  memory options  */
#define MEM_ALIGNMENT           4                        /* should be set to the alignment of the CPU for which lwIP
                                                            is compiled. 4 byte alignment -> define MEM_ALIGNMENT 
                                                            to 4, 2 byte alignment -> define MEM_ALIGNMENT to 2 */

#define MEM_SIZE                (8*1024)                 /* the size of the heap memory, if the application will 
                                                            send a lot of data that needs to be copied, this should
                                                            be set high */

#define MEMP_NUM_PBUF           10                       /* the number of memp struct pbufs. If the application
                                                            sends a lot of data out of ROM (or other static memory),
                                                            this should be set high */

#define MEMP_NUM_UDP_PCB        6                        /* the number of UDP protocol control blocks, one
                                                            per active UDP "connection" */

#define MEMP_NUM_TCP_PCB        10                       /* the number of simulatenously active TCP connections */

#define MEMP_NUM_TCP_PCB_LISTEN 6                        /* the number of listening TCP connections */

#define MEMP_NUM_TCP_SEG        12                       /* the number of simultaneously queued TCP segments */

#define MEMP_NUM_SYS_TIMEOUT    10                        /* the number of simulateously active timeouts */

/* Pbuf options */
#define PBUF_POOL_SIZE          10                       /* the number of buffers in the pbuf pool */
#define PBUF_POOL_BUFSIZE       1500                     /* the size of each pbuf in the pbuf pool */

/* TCP options */
#define LWIP_TCP                1
#define TCP_TTL                 255

#define TCP_QUEUE_OOSEQ         0                        /* controls if TCP should queue segments that arrive out of
                                                            order, Define to 0 if your device is low on memory. */

/* DHCP options */
#define LWIP_DHCP               1                        /* define to 1 if you want DHCP configuration of interfaces,
                                                            DHCP is not implemented in lwIP 0.5.1, however, so
                                                            turning this on does currently not work. */
#define LWIP_DHCP_DOES_ACD_CHECK 0

/* UDP options, required for DHCP */
#define LWIP_UDP                1
#define UDP_TTL                 255

/* checksum options */
#define CHECKSUM_BY_HARDWARE                             /* computing and verifying the IP, UDP, TCP and ICMP
                                                            checksums by hardware */
/* provide access to stats */
#define LWIP_STATS              0

/* sequential layer options */
#define LWIP_NETCONN            0                        /* set to 1 to enable netconn API (require to use api_lib.c) */

/* socket options */
#define LWIP_SOCKET             0                        /* set to 1 to enable socket API (require to use sockets.c) */

/* enable ethernet bridge */
#define LWIP_ETHERNET           1

/* support hostname */
#define LWIP_NETIF_HOSTNAME 	1

/* No support for HTTP0.9 */
#define LWIP_HTTPD_SUPPORT_V09  0


/* Non-bootloader options */
#ifndef BOOTLOADER

#define LWIP_ICMP               		1
#define IP_SOF_BROADCAST        		1
#define IP_SOF_BROADCAST_RECV   		1
#define LWIP_BROADCAST_PING     		1
#define LWIP_MULTICAST_PING     		1
#define LWIP_DNS_SECURE         		7

#define LWIP_HTTPD_SUPPORT_REST 		1
#define LWIP_HTTPD_DYNAMIC_HEADERS 		1
#define LWIP_HTTPD_DYNAMIC_FILE_READ 	1

#define HTTPD_FSDATA_FILE "../fsdata.c"

#endif  // #ifndef BOOTLOADER

/* Bootloader options */
#ifdef BOOTLOADER

#define LWIP_HTTPD_SUPPORT_POST			1
#define LWIP_ICMP              		 	0

#define HTTPD_FSDATA_FILE "../fsdata_bootloader.c"

#endif  // #ifdef BOOTLOADER

#ifdef CHECKSUM_BY_HARDWARE
    /* CHECKSUM_GEN_IP==0: generate checksums by hardware for outgoing IP packets.*/
    #define CHECKSUM_GEN_IP                 0
    /* CHECKSUM_GEN_UDP==0: generate checksums by hardware for outgoing UDP packets.*/
    #define CHECKSUM_GEN_UDP                0
    /* CHECKSUM_GEN_TCP==0: generate checksums by hardware for outgoing TCP packets.*/
    #define CHECKSUM_GEN_TCP                0 
    /* CHECKSUM_CHECK_IP==0: check checksums by hardware for incoming IP packets.*/
    #define CHECKSUM_CHECK_IP               0
    /* CHECKSUM_CHECK_UDP==0: check checksums by hardware for incoming UDP packets.*/
    #define CHECKSUM_CHECK_UDP              0
    /* CHECKSUM_CHECK_TCP==0: check checksums by hardware for incoming TCP packets.*/
    #define CHECKSUM_CHECK_TCP              0
//    #define CHECKSUM_GEN_ICMP               0
#else
    /* CHECKSUM_GEN_IP==1: generate checksums in software for outgoing IP packets.*/
    #define CHECKSUM_GEN_IP                 1
    /* CHECKSUM_GEN_UDP==1: generate checksums in software for outgoing UDP packets.*/
    #define CHECKSUM_GEN_UDP                1
    /* CHECKSUM_GEN_TCP==1: generate checksums in software for outgoing TCP packets.*/
    #define CHECKSUM_GEN_TCP                1
    /* CHECKSUM_CHECK_IP==1: check checksums in software for incoming IP packets.*/
    #define CHECKSUM_CHECK_IP               1
    /* CHECKSUM_CHECK_UDP==1: check checksums in software for incoming UDP packets.*/
    #define CHECKSUM_CHECK_UDP              1
    /* CHECKSUM_CHECK_TCP==1: check checksums in software for incoming TCP packets.*/
    #define CHECKSUM_CHECK_TCP              1
//    #define CHECKSUM_GEN_ICMP               1
#endif

/* Lwip debug options */
//#define LWIP_DEBUG 1
//#define LWIP_PLATFORM_DIAG(x)   do {printf x;} while(0)
//#define HTTPD_DEBUG LWIP_DBG_ON
//#define NETIF_DEBUG LWIP_DBG_ON
//#define SOCKETS_DEBUG LWIP_DBG_ON
//#define ICMP_DEBUG LWIP_DBG_ON
//#define INET_DEBUG LWIP_DBG_ON
//#define IP_DEBUG LWIP_DBG_ON
//#define UDP_DEBUG LWIP_DBG_ON
//#define DHCP_DEBUG LWIP_DBG_ON
//#define TCPIP_DEBUG LWIP_DBG_ON
//#define PING_DEBUG LWIP_DBG_ON


#endif /* LWIPOPTS_H */
