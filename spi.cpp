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

extern "C" {
#include "gd32f10x.h"
}

#include "./spi.h"
#include "./control.h"
#include "./perf.h"

extern "C" {

__attribute__((used))
void DMA0_Channel2_IRQHandler() {
    if(dma_interrupt_flag_get(DMA0, DMA_CH2, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_FTF);   
		lightkraken::PerfMeasure perf(lightkraken::PerfMeasure::SLOT_SPI_INTERRUPT);
        lightkraken::Control::instance().syncFromInterrupt(lightkraken::SPI_0::instance());
    }
}

__attribute__((used))
void DMA1_Channel1_IRQHandler() {
    if(dma_interrupt_flag_get(DMA1, DMA_CH1, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(DMA1, DMA_CH1, DMA_INT_FLAG_FTF);         
		lightkraken::PerfMeasure perf(lightkraken::PerfMeasure::SLOT_SPI_INTERRUPT);
        lightkraken::Control::instance().syncFromInterrupt(lightkraken::SPI_2::instance());
    }
}

}

namespace lightkraken {

SPI_0 &SPI_0::instance() {
    static SPI_0 spi0;
    if (!spi0.initialized) {
        spi0.initialized = true;
        spi0.init();
    }
    return spi0;
}

bool SPI_0::busy() const {
	if(!dma_flag_get(DMA0, DMA_CH2, DMA_FLAG_FTF) ||
		dma_transfer_number_get(DMA0, DMA_CH2)) {
		return true;
	}
	return false;
}

void SPI_0::transfer(const uint8_t *buf, size_t len, bool wantsSCLK) {

    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    gpio_bit_set(GPIOA, GPIO_PIN_10);
    
	if (active) {
		if (busy()) {
			scheduled = true;
			return;
		}
	}

    dma_channel_disable(DMA0, DMA_CH2);
    active = false;

    if (cbuf != buf || clen != len || wantsSCLK != sclk) {
        cbuf = buf;
        clen = len;
        sclk = wantsSCLK;
        dma_setup();
    }

    dma_channel_enable(DMA0, DMA_CH2);
    active = true;
}

void SPI_0::update() {
	if (scheduled) {
		scheduled = false;
		transfer(cbuf, clen, sclk);
	}
}

void SPI_0::dma_setup() {
    spi_disable(SPI0);

    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    gpio_bit_set(GPIOA, GPIO_PIN_10);
    
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    
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
    dma_init_struct.memory_addr  = (uint32_t)cbuf;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority     = DMA_PRIORITY_LOW;
    dma_init_struct.number       = clen;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA0, DMA_CH2, &dma_init_struct);
    dma_circulation_disable(DMA0, DMA_CH2);
    //dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF);
    dma_memory_to_memory_disable(DMA0, DMA_CH2);
    
    nvic_irq_enable(DMA0_Channel2_IRQn, 0, 0);

    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);
}

void SPI_0::init() {
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_SPI0);
    rcu_periph_clock_enable(RCU_DMA0);

    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    gpio_pin_remap_config(GPIO_SPI0_REMAP, ENABLE);
}

SPI_2 &SPI_2::instance() {
    static SPI_2 spi2;
    if (!spi2.initialized) {
        spi2.initialized = true;
        spi2.init();
    }
    return spi2;
}

bool SPI_2::busy() const {
	if(!dma_flag_get(DMA1, DMA_CH1, DMA_FLAG_FTF) ||
		dma_transfer_number_get(DMA1, DMA_CH1)) {
		return true;
	}
	return false;
}

void SPI_2::transfer(const uint8_t *buf, size_t len, bool wantsSCLK) {

    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_bit_set(GPIOB, GPIO_PIN_9);
    
	if (active) {
		if(busy()) {
			scheduled = true;
			return;
		}
	}
    
    dma_channel_disable(DMA1, DMA_CH1);
    active = false;

    if (cbuf != buf || clen != len || wantsSCLK != sclk) {
        cbuf = buf;
        clen = len;
        sclk = wantsSCLK;
        dma_setup();
    }

    dma_channel_enable(DMA1, DMA_CH1);
    active = true;

}

void SPI_2::update() {
	if (scheduled) {
		scheduled = false;
		transfer(cbuf, clen, sclk);
	}
}

void SPI_2::dma_setup() {
    spi_disable(SPI2);

    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_bit_set(GPIOB, GPIO_PIN_9);
    
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    
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
    dma_init_struct.memory_addr  = (uint32_t)cbuf;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority     = DMA_PRIORITY_LOW;
    dma_init_struct.number       = clen;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA1, DMA_CH1, &dma_init_struct);
    dma_circulation_disable(DMA1, DMA_CH1);
    //dma_interrupt_enable(DMA1, DMA_CH1, DMA_INT_FTF);
    dma_memory_to_memory_disable(DMA1, DMA_CH1);

    nvic_irq_enable(DMA1_Channel1_IRQn, 0, 0);

    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);
}

void SPI_2::init() {
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_DMA1);

    gpio_pin_remap_config(GPIO_SPI2_REMAP, ENABLE);

}

}  // namespace lightkraken {
