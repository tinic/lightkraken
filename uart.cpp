#include <stdio.h>
#include <stdint.h>

extern "C" {
#include "gd32f10x.h"
#include "cmsis_gcc.h"

int __io_putchar(int ch);
int _write(int file, char *ptr, int len);

}

#include "./main.h"
#include "./uart.h"

int __io_putchar(int ch){
    lightguy::UART::instance().transmit(ch);
    return(ch);
}

__attribute__((used)) // required for -flto
int _write(int, char *ptr, int len) {

    __disable_irq();

    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++) {
        __io_putchar(*ptr++);
    }

    __enable_irq();

    return len;
}

namespace lightguy {

UART &UART::instance() {
    static UART uart;
    if (!uart.initialized) {
        uart.initialized = true;
        uart.init();
    }
    return uart;
}

void UART::transmit(int ch) {
    usart_data_transmit(USART0, (uint8_t)ch);
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
}

void UART::init() {

    __disable_irq();

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

    usart_enable(USART0);

    __enable_irq();
    
}

}  // namespace lightguy {
