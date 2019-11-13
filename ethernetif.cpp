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
}; // extern "C" {

#include "./main.h"
#include "./ethernetif.h"
#include "./netconf.h"
#include "./status.h"

extern "C" {
    extern enet_descriptors_struct rxdesc_tab[ENET_RXBUF_NUM];
    extern enet_descriptors_struct txdesc_tab[ENET_TXBUF_NUM];

    extern uint8_t rx_buff[ENET_RXBUF_NUM][ENET_RXBUF_SIZE]; 
    extern uint8_t tx_buff[ENET_TXBUF_NUM][ENET_TXBUF_SIZE]; 

    extern enet_descriptors_struct  *dma_current_txdesc;
    extern enet_descriptors_struct  *dma_current_rxdesc;
};

namespace lightkraken {

EthernetIf &EthernetIf::instance() {
    static EthernetIf ethernetif;
    if (!ethernetif.initialized) {
        ethernetif.initialized = true;
        ethernetif.init();
    }
    return ethernetif;
}

void EthernetIf::init() {
    DEBUG_PRINTF(("ENET hardware init.\n"));

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

    /* PB15: nRST, pull low*/
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);    
    gpio_bit_reset(GPIOB, GPIO_PIN_15);

    /* enable SYSCFG clock */
    rcu_periph_clock_enable(RCU_AF);

    rcu_pll2_config(RCU_PLL2_MUL10);
    rcu_osci_on(RCU_PLL2_CK);
    rcu_osci_stab_wait(RCU_PLL2_CK);
    /* get 50MHz from CK_PLL2 on CKOUT0 pin (PA8) to clock the PHY */
    rcu_ckout0_config(RCU_CKOUT0SRC_CKPLL2);

    gpio_ethernet_phy_select(GPIO_ENET_PHY_RMII);

    /* PA1: ETH_RMII_REF_CLK */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    /* PA2: ETH_MDIO */    
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    /* PA7: ETH_RMII_CRS_DV */    
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_7);

    /* PC1: ETH_MDC */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    /* PC4: ETH_RMII_RXD0 */    
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    /* PC5: ETH_RMII_RXD1 */    
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_5);

    /* PB11: ETH_RMII_TX_EN */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    /* PB12: ETH_RMII_TXD0 */    
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    /* PB13: ETH_RMII_TXD1 */    
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);    

    /* PB14: NINT */
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_14);    

    enet_delay(10000);

    /* PB15: nRST, set high*/
    gpio_bit_set(GPIOB, GPIO_PIN_15);

    DEBUG_PRINTF(("ENET hardware up.\n"));

    /* enable ethernet clock  */
    rcu_periph_clock_enable(RCU_ENET);
    rcu_periph_clock_enable(RCU_ENETTX);
    rcu_periph_clock_enable(RCU_ENETRX);
    
    /* reset ethernet on AHB bus */
    enet_deinit();

    ErrStatus reval_state = enet_software_reset();
    if(reval_state == ERROR){
        while(1) {
        }
    }

	enet_initpara_config(FILTER_OPTION, ENET_MULTICAST_FILTER_NONE);
    uint32_t enet_init_status = enet_init(
    	ENET_AUTO_NEGOTIATION, 
    	ENET_AUTOCHECKSUM_DROP_FAILFRAMES, 
    	ENET_BROADCAST_FRAMES_PASS /* | ENET_PROMISCUOUS_MODE*/ );
    if (enet_init_status == 0){
        while(1) {
        }
    }
    
#ifndef BOOTLOADER
    StatusLED::instance().setEnetUp();
#endif  // #ifndef BOOTLOADER

    DEBUG_PRINTF(("ENET MAC config done.\n"));
}

void EthernetIf::low_level_init(struct netif *netif, uint32_t mac_addr) {
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    netif->hwaddr[0] =  0x1E;
    netif->hwaddr[1] =  0xD5;

    netif->hwaddr[2] =  ( mac_addr >> 24 ) & 0xFF;
    netif->hwaddr[3] =  ( mac_addr >> 16 ) & 0xFF;
    netif->hwaddr[4] =  ( mac_addr >>  8 ) & 0xFF;
    netif->hwaddr[5] =  ( mac_addr >>  0 ) & 0xFF;
    
    enet_mac_address_set(ENET_MAC_ADDRESS0, netif->hwaddr);

    netif->mtu = 1500;

    netif->flags = NETIF_FLAG_IGMP | NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    enet_descriptors_chain_init(ENET_DMA_TX);
    enet_descriptors_chain_init(ENET_DMA_RX);

    for(uint32_t i=0; i<ENET_RXBUF_NUM; i++){ 
        enet_desc_receive_complete_bit_enable(&rxdesc_tab[i]);
    }

    for(uint32_t i=0; i < ENET_TXBUF_NUM; i++){
        enet_transmit_checksum_config(&txdesc_tab[i], ENET_CHECKSUM_TCPUDPICMP_FULL);
    }

    enet_enable();
}

err_t EthernetIf::low_level_output(struct netif *netif, struct pbuf *p) {
    (void)netif;
    
    while((uint32_t)RESET != (dma_current_txdesc->status & ENET_TDES0_DAV)) { }  
    
    uint8_t *buffer = (uint8_t *)(enet_desc_information_get(dma_current_txdesc, TXDESC_BUFFER_1_ADDR));
    
    int32_t framelength = 0;
    for(struct pbuf *q = p; q != NULL; q = q->next){ 
        memcpy((uint8_t *)&buffer[framelength], q->payload, q->len);
        framelength = framelength + q->len;
    }
    
    ENET_NOCOPY_FRAME_TRANSMIT(framelength);

    return ERR_OK;
}

struct pbuf *EthernetIf::low_level_input(struct netif *netif) {
    (void)netif;

    u16_t len = enet_desc_information_get(dma_current_rxdesc, RXDESC_FRAME_LENGTH);
    uint8_t *buffer = (uint8_t *)(enet_desc_information_get(dma_current_rxdesc, RXDESC_BUFFER_1_ADDR));
    
    struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    if (p != NULL){
        int32_t l = 0;
        for (struct pbuf *q = p; q != NULL; q = q->next){ 
            memcpy((uint8_t *)q->payload, (u8_t*)&buffer[l], q->len);
            l = l + q->len;
        }    
    }

    ENET_NOCOPY_FRAME_RECEIVE();

    return p;
}

err_t EthernetIf::ethernetif_input(struct netif *netif) {
    struct pbuf *p = low_level_input(netif);

    if (p == NULL) {
        return ERR_MEM;
    }

    err_t err = netif->input(p, netif);
    if (err != ERR_OK){
        pbuf_free(p);
        p = NULL;
    }
    
    return err;
}


err_t EthernetIf::ethernetif_init(struct netif *netif) {

    uint32_t uid[3];
    uid[0] = instance().get_uid0();
    uid[1] = instance().get_uid1();
    uid[2] = instance().get_uid2();
    uint32_t mac_addr  = instance().murmur3_32(reinterpret_cast<uint8_t *>(&uid[0]), sizeof(uid), 0x66cf8031);

    static const char hostname_base[] = "lightkraken-";
    static const char hex_table[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',};
    static char hostname[sizeof(hostname_base)+8];
    memset(hostname, 0, sizeof(hostname));
    strcpy(hostname, hostname_base);
    for (size_t c=0; c<8; c++) {
        hostname[c + sizeof(hostname_base) - 1] = hex_table[(mac_addr>>(32-((c+1)*4)))&0xF];
    }
    
    DEBUG_PRINTF(("Hostname is %s\n", hostname));
    netif->hostname = hostname;

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;

    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    instance().low_level_init(netif, mac_addr);

    return ERR_OK;
}

uint32_t EthernetIf::get_uid0() const { 
    return *reinterpret_cast<uint32_t*>(0x1FFFF7E8); 
}

uint32_t EthernetIf::get_uid1() const { 
    return *reinterpret_cast<uint32_t*>(0x1FFFF7EC); 
}

uint32_t EthernetIf::get_uid2() const  { 
    return *reinterpret_cast<uint32_t*>(0x1FFFF7F0); 
}

uint32_t EthernetIf::murmur3_32(const uint8_t* key, size_t len, uint32_t seed) const {
    uint32_t h = seed;
    if (len > 3) {
        size_t i = len >> 2;
        do {
            uint32_t k;
            memcpy(&k, key, sizeof(uint32_t));
            key += sizeof(uint32_t);
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h = h * 5 + 0xe6546b64;
        } while (--i);
    }
    if (len & 3) {
        size_t i = len & 3;
        uint32_t k = 0;
        do {
            k <<= 8;
            k |= key[i - 1];
        } while (--i);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
    }
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

}
