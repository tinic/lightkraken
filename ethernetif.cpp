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
#include <string.h>
#include <stdint.h>

extern "C" {
#include "gd32f10x.h"
#include "cmsis_gcc.h"

#include "lwip/mem.h"
#include "netif/etharp.h"

#include "ethernetif.h"
#include "netconf.h"
}; // extern "C" {

extern enet_descriptors_struct  rxdesc_tab[ENET_RXBUF_NUM], txdesc_tab[ENET_TXBUF_NUM];

extern uint8_t rx_buff[ENET_RXBUF_NUM][ENET_RXBUF_SIZE]; 
extern uint8_t tx_buff[ENET_TXBUF_NUM][ENET_TXBUF_SIZE]; 

extern enet_descriptors_struct  *dma_current_txdesc;
extern enet_descriptors_struct  *dma_current_rxdesc;

enet_descriptors_struct  ptp_txstructure[ENET_TXBUF_NUM];
enet_descriptors_struct  ptp_rxstructure[ENET_RXBUF_NUM];

static void low_level_init(struct netif *netif) {
    unsigned int i; 

    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    netif->hwaddr[0] =  MAC_ADDR0;
    netif->hwaddr[1] =  MAC_ADDR1;
    netif->hwaddr[2] =  MAC_ADDR2;
    netif->hwaddr[3] =  MAC_ADDR3;
    netif->hwaddr[4] =  MAC_ADDR4;
    netif->hwaddr[5] =  MAC_ADDR5;
    
    enet_mac_address_set(ENET_MAC_ADDRESS0, netif->hwaddr);

    netif->mtu = 1500;

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    enet_descriptors_chain_init(ENET_DMA_TX);
    enet_descriptors_chain_init(ENET_DMA_RX);

	for(i=0; i<ENET_RXBUF_NUM; i++){ 
	   enet_desc_receive_complete_bit_enable(&rxdesc_tab[i]);
	}

    for(i=0; i < ENET_TXBUF_NUM; i++){
        enet_transmit_checksum_config(&txdesc_tab[i], ENET_CHECKSUM_TCPUDPICMP_FULL);
    }

    enet_enable();
}

static err_t low_level_output(struct netif *netif, struct pbuf *p) {
    struct pbuf *q;
    int framelength = 0;
    uint8_t *buffer;

	(void)netif;
    
    while((uint32_t)RESET != (dma_current_txdesc->status & ENET_TDES0_DAV)){
    }  
    
    buffer = (uint8_t *)(enet_desc_information_get(dma_current_txdesc, TXDESC_BUFFER_1_ADDR));
    
    for(q = p; q != NULL; q = q->next){ 
        memcpy((uint8_t *)&buffer[framelength], q->payload, q->len);
        framelength = framelength + q->len;
    }
    
    ENET_NOCOPY_FRAME_TRANSMIT(framelength);

    return ERR_OK;
}

static struct pbuf * low_level_input(struct netif *netif) {
    struct pbuf *p, *q;
    u16_t len;
    int l =0;
    uint8_t *buffer;
     
    (void)netif;
     
    p = NULL;
    
    len = enet_desc_information_get(dma_current_rxdesc, RXDESC_FRAME_LENGTH);
    buffer = (uint8_t *)(enet_desc_information_get(dma_current_rxdesc, RXDESC_BUFFER_1_ADDR));
    
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    
    if (p != NULL){
        for (q = p; q != NULL; q = q->next){ 
            memcpy((uint8_t *)q->payload, (u8_t*)&buffer[l], q->len);
            l = l + q->len;
        }    
    }
  
    ENET_NOCOPY_FRAME_RECEIVE();

    return p;
}

err_t ethernetif_input(struct netif *netif) {
    err_t err;
    struct pbuf *p;

    p = low_level_input(netif);

    if (p == NULL) return ERR_MEM;

    err = netif->input(p, netif);
    
    if (err != ERR_OK){
        pbuf_free(p);
        p = NULL;
    }
    return err;
}

err_t ethernetif_init(struct netif *netif) {
    LWIP_ASSERT("netif != NULL", (netif != NULL));
  
    /* Initialize interface hostname */
    netif->hostname = "Gigadevice.COM_lwip";

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;

    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    low_level_init(netif);

    return ERR_OK;
}
