#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
}

#include "./spi.h"

namespace lightguy {

SPI_0 &SPI_0::instance() {
	static SPI_0 spi0;
	if (!spi0.initialized) {
		spi0.initialized = true;
		spi0.init();
	}
	return spi0;
}

void SPI_0::transfer(const uint8_t *buf, size_t len) {
    if (active) {
        if(!dma_flag_get(DMA0, DMA_CH2, DMA_FLAG_FTF) ||
            dma_transfer_number_get(DMA0, DMA_CH2)) {
            scheduled = true;
            return;
        }
    }

    dma_channel_disable(DMA0, DMA_CH2);
    active = false;
  
    if (cbuf != buf || clen != len) {
        cbuf = buf;
        clen = len;
        dma_setup(buf, len);
    }
  
    dma_channel_enable(DMA0, DMA_CH2);
    active = true;
}

void SPI_0::update() {
    if (scheduled) {
        scheduled = false;
        transfer(cbuf, clen);
    }
}

void SPI_0::dma_setup(const uint8_t *buf, size_t len) {
    spi_parameter_struct spi_init_struct;
    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_32;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);
    
    spi_enable(SPI0);

    dma_deinit(DMA0, DMA_CH2) ;
    dma_parameter_struct dma_init_struct;
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI0);
    dma_init_struct.memory_addr  = (uint32_t)buf;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority     = DMA_PRIORITY_LOW;
    dma_init_struct.number       = len;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA0, DMA_CH2, &dma_init_struct);
    dma_circulation_disable(DMA0, DMA_CH2);
    dma_memory_to_memory_disable(DMA0, DMA_CH2);

    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);
}

void SPI_0::init() {
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_SPI0);
    rcu_periph_clock_enable(RCU_DMA0);

    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    gpio_pin_remap_config(GPIO_SPI0_REMAP, ENABLE);
    
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_3);
}

SPI_2 &SPI_2::instance() {
	static SPI_2 spi2;
	if (!spi2.initialized) {
		spi2.initialized = true;
		spi2.init();
	}
	return spi2;
}

void SPI_2::transfer(const uint8_t *buf, size_t len) {

    if (active) {
        if(!dma_flag_get(DMA1, DMA_CH1, DMA_FLAG_FTF) ||
            dma_transfer_number_get(DMA1, DMA_CH1)) {
            scheduled = true;
            return;
        }
    }
    
    dma_channel_disable(DMA1, DMA_CH1);
    active = false;
  
    if (cbuf != buf || clen != len) {
        cbuf = buf;
        clen = len;
        dma_setup(buf, len);
    }
  
    dma_channel_enable(DMA1, DMA_CH1);
    active = true;

}

void SPI_2::update() {
    if (scheduled) {
        scheduled = false;
        transfer(cbuf, clen);
    }
}

void SPI_2::dma_setup(const uint8_t *buf, size_t len) {
    spi_parameter_struct spi_init_struct;
    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_16;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI2, &spi_init_struct);
    
    spi_enable(SPI2);

    dma_deinit(DMA1, DMA_CH1) ;
    dma_parameter_struct dma_init_struct;
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);
    dma_init_struct.memory_addr  = (uint32_t)buf;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority     = DMA_PRIORITY_LOW;
    dma_init_struct.number       = len;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA1, DMA_CH1, &dma_init_struct);
    dma_circulation_disable(DMA1, DMA_CH1);
    dma_memory_to_memory_disable(DMA1, DMA_CH1);

    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);
}

void SPI_2::init() {
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_DMA1);

    gpio_pin_remap_config(GPIO_SPI2_REMAP, ENABLE);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_12);
}

}  // namespace lightguy {
