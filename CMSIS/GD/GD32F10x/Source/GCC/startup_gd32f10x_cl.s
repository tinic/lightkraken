  .syntax unified
  .cpu cortex-m3
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

.word _sidata
.word _sdata
.word _edata
.word _sbss
.word _ebss

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:

  movs r1, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r3, =_sidata
  ldr r3, [r3, r1]
  str r3, [r0, r1]
  adds r1, r1, #4

LoopCopyDataInit:
  ldr r0, =_sdata
  ldr r3, =_edata
  adds r2, r0, r1
  cmp r2, r3
  bcc CopyDataInit
  ldr r2, =_sbss
  b LoopFillZerobss

FillZerobss:
  movs r3, #0
  str r3, [r2], #4

LoopFillZerobss:
  ldr r3, = _ebss
  cmp r2, r3
  bcc FillZerobss

  bl  SystemInit
  bl __libc_init_array
  bl main
  bx lr
  .size Reset_Handler, .-Reset_Handler
  
  .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler

  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
/* 00 */  .word _estack
/* 01 */  .word Reset_Handler
/* 02 */  .word NMI_Handler
/* 03 */  .word HardFault_Handler
/* 04 */  .word MemManage_Handler
/* 05 */  .word BusFault_Handler
/* 06 */  .word UsageFault_Handler
/* 07 */  .word 0
/* 08 */  .word 0
/* 09 */  .word 0
/* 10 */  .word 0
/* 11 */  .word SVC_Handler
/* 12 */  .word DebugMon_Handler
/* 13 */  .word 0
/* 14 */  .word PendSV_Handler
/* 15 */  .word SysTick_Handler
/* 16 */  .word WWDG_IRQHandler
/* 17 */  .word LVD_IRQHandler
/* 18 */  .word TAMPER_IRQHandler
/* 19 */  .word RTC_IRQHandler
/* 20 */  .word FMC_IRQHandler
/* 21 */  .word RCU_IRQHandler
/* 22 */  .word EXTI0_IRQHandler
/* 23 */  .word EXTI1_IRQHandler
/* 24 */  .word EXTI2_IRQHandler
/* 25 */  .word EXTI3_IRQHandler
/* 26 */  .word EXTI4_IRQHandler
/* 27 */  .word DMA0_Channel0_IRQHandler
/* 28 */  .word DMA0_Channel1_IRQHandler
/* 29 */  .word DMA0_Channel2_IRQHandler
/* 30 */  .word DMA0_Channel3_IRQHandler
/* 31 */  .word DMA0_Channel4_IRQHandler
/* 32 */  .word DMA0_Channel5_IRQHandler
/* 33 */  .word DMA0_Channel6_IRQHandler
/* 34 */  .word ADC0_1_IRQHandler
/* 35 */  .word CAN0_TX_IRQHandler
/* 36 */  .word CAN0_RX0_IRQHandler
/* 37 */  .word CAN0_RX1_IRQHandler
/* 38 */  .word CAN0_EWMC_IRQHandler
/* 39 */  .word EXTI5_9_IRQHandler
/* 40 */  .word TIMER0_BRK_IRQHandler
/* 41 */  .word TIMER0_UP_IRQHandler
/* 42 */  .word TIMER0_TRG_CMT_IRQHandler
/* 43 */  .word TIMER0_Channel_IRQHandler
/* 44 */  .word TIMER1_IRQHandler
/* 45 */  .word TIMER2_IRQHandler
/* 46 */  .word TIMER3_IRQHandler
/* 47 */  .word I2C0_EV_IRQHandler
/* 48 */  .word I2C0_ER_IRQHandler
/* 49 */  .word I2C1_EV_IRQHandler
/* 50 */  .word I2C1_ER_IRQHandler
/* 51 */  .word SPI0_IRQHandler
/* 52 */  .word SPI1_IRQHandler
/* 53 */  .word USART0_IRQHandler
/* 54 */  .word USART1_IRQHandler
/* 55 */  .word USART2_IRQHandler
/* 56 */  .word EXTI10_15_IRQHandler
/* 57 */  .word RTC_Alarm_IRQHandler
/* 58 */  .word USBFS_WKUP_IRQHandler
/* 59 */  .word TIMER7_BRK_IRQHandler
/* 60 */  .word TIMER7_UP_IRQHandler
/* 61 */  .word TIMER7_TRG_CMT_IRQHandler
/* 62 */  .word TIMER7_Channel_IRQHandler
/* 63 */  .word 0
/* 64 */  .word EXMC_IRQHandler
/* 65 */  .word 0
/* 66 */  .word TIMER4_IRQHandler
/* 67 */  .word SPI2_IRQHandler
/* 68 */  .word UART3_IRQHandler
/* 69 */  .word UART4_IRQHandler
/* 70 */  .word TIMER5_IRQHandler
/* 71 */  .word TIMER6_IRQHandler
/* 72 */  .word DMA1_Channel0_IRQHandler
/* 73 */  .word DMA1_Channel1_IRQHandler
/* 74 */  .word DMA1_Channel2_IRQHandler
/* 75 */  .word DMA1_Channel3_IRQHandler
/* 76 */  .word DMA1_Channel4_IRQHandler
/* 77 */  .word ENET_IRQHandler
/* 78 */  .word ENET_WKUP_IRQHandler
/* 79 */  .word CAN1_TX_IRQHandler
/* 80 */  .word CAN1_RX0_IRQHandler
/* 81 */  .word CAN1_RX1_IRQHandler
/* 82 */  .word CAN1_EWMC_IRQHandler
/* 83 */  .word USBFS_IRQHandler

  .weak  NMI_Handler
  .thumb_set NMI_Handler,Default_Handler

  .weak  HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler

  .weak  MemManage_Handler
  .thumb_set MemManage_Handler,Default_Handler

  .weak  BusFault_Handler
  .thumb_set BusFault_Handler,Default_Handler

  .weak  UsageFault_Handler
  .thumb_set UsageFault_Handler,Default_Handler

  .weak  SVC_Handler
  .thumb_set SVC_Handler,Default_Handler

  .weak  DebugMon_Handler
  .thumb_set DebugMon_Handler,Default_Handler

  .weak  PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler

  .weak  SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler

  .weak  WWDG_IRQHandler
  .thumb_set WWDG_IRQHandler,Default_Handler

  .weak  LVD_IRQHandler
  .thumb_set LVD_IRQHandler,Default_Handler

  .weak  TAMPER_IRQHandler
  .thumb_set TAMPER_IRQHandler,Default_Handler

  .weak  RTC_IRQHandler
  .thumb_set RTC_IRQHandler,Default_Handler

  .weak  FMC_IRQHandler
  .thumb_set FMC_IRQHandler,Default_Handler

  .weak  RCU_IRQHandler
  .thumb_set RCU_IRQHandler,Default_Handler

  .weak  EXTI0_IRQHandler
  .thumb_set EXTI0_IRQHandler,Default_Handler

  .weak  EXTI1_IRQHandler
  .thumb_set EXTI1_IRQHandler,Default_Handler

  .weak  EXTI2_IRQHandler
  .thumb_set EXTI2_IRQHandler,Default_Handler

  .weak  EXTI3_IRQHandler
  .thumb_set EXTI3_IRQHandler,Default_Handler

  .weak  EXTI4_IRQHandler
  .thumb_set EXTI4_IRQHandler,Default_Handler

  .weak  DMA0_Channel0_IRQHandler
  .thumb_set DMA0_Channel0_IRQHandler,Default_Handler

  .weak  DMA0_Channel1_IRQHandler
  .thumb_set DMA0_Channel1_IRQHandler,Default_Handler

  .weak  DMA0_Channel2_IRQHandler
  .thumb_set DMA0_Channel2_IRQHandler,Default_Handler

  .weak  DMA0_Channel3_IRQHandler
  .thumb_set DMA0_Channel3_IRQHandler,Default_Handler

  .weak  DMA0_Channel4_IRQHandler
  .thumb_set DMA0_Channel4_IRQHandler,Default_Handler

  .weak  DMA0_Channel5_IRQHandler
  .thumb_set DMA0_Channel5_IRQHandler,Default_Handler

  .weak  DMA0_Channel6_IRQHandler
  .thumb_set DMA0_Channel6_IRQHandler,Default_Handler

  .weak  ADC0_1_IRQHandler
  .thumb_set ADC0_1_IRQHandler,Default_Handler

  .weak  CAN0_TX_IRQHandler
  .thumb_set CAN0_TX_IRQHandler,Default_Handler

  .weak  CAN0_RX0_IRQHandler
  .thumb_set CAN0_RX0_IRQHandler,Default_Handler

  .weak  CAN0_RX1_IRQHandler
  .thumb_set CAN0_RX1_IRQHandler,Default_Handler

  .weak  CAN0_EWMC_IRQHandler
  .thumb_set CAN0_EWMC_IRQHandler,Default_Handler

  .weak  EXTI5_9_IRQHandler
  .thumb_set EXTI5_9_IRQHandler,Default_Handler

  .weak  TIMER0_BRK_IRQHandler
  .thumb_set TIMER0_BRK_IRQHandler,Default_Handler

  .weak  TIMER0_UP_IRQHandler
  .thumb_set TIMER0_UP_IRQHandler,Default_Handler

  .weak  TIMER0_TRG_CMT_IRQHandler
  .thumb_set TIMER0_TRG_CMT_IRQHandler,Default_Handler

  .weak  TIMER0_Channel_IRQHandler
  .thumb_set TIMER0_Channel_IRQHandler,Default_Handler

  .weak  TIMER1_IRQHandler
  .thumb_set TIMER1_IRQHandler,Default_Handler

  .weak  TIMER2_IRQHandler
  .thumb_set TIMER2_IRQHandler,Default_Handler

  .weak  TIMER3_IRQHandler
  .thumb_set TIMER3_IRQHandler,Default_Handler

  .weak  I2C0_EV_IRQHandler
  .thumb_set I2C0_EV_IRQHandler,Default_Handler

  .weak  I2C0_ER_IRQHandler
  .thumb_set I2C0_ER_IRQHandler,Default_Handler

  .weak  I2C1_EV_IRQHandler
  .thumb_set I2C1_EV_IRQHandler,Default_Handler

  .weak  I2C1_ER_IRQHandler
  .thumb_set I2C1_ER_IRQHandler,Default_Handler

  .weak  SPI0_IRQHandler
  .thumb_set SPI0_IRQHandler,Default_Handler

  .weak  SPI1_IRQHandler
  .thumb_set SPI1_IRQHandler,Default_Handler

  .weak  USART0_IRQHandler
  .thumb_set USART0_IRQHandler,Default_Handler

  .weak  USART1_IRQHandler
  .thumb_set USART1_IRQHandler,Default_Handler

  .weak  USART2_IRQHandler
  .thumb_set USART2_IRQHandler,Default_Handler

  .weak  EXTI10_15_IRQHandler
  .thumb_set EXTI10_15_IRQHandler,Default_Handler

  .weak  RTC_Alarm_IRQHandler
  .thumb_set RTC_Alarm_IRQHandler,Default_Handler

  .weak  USBFS_WKUP_IRQHandler
  .thumb_set USBFS_WKUP_IRQHandler,Default_Handler

  .weak  TIMER7_BRK_IRQHandler
  .thumb_set TIMER7_BRK_IRQHandler,Default_Handler

  .weak  TIMER7_UP_IRQHandler
  .thumb_set TIMER7_UP_IRQHandler,Default_Handler

  .weak  TIMER7_TRG_CMT_IRQHandler
  .thumb_set TIMER7_TRG_CMT_IRQHandler,Default_Handler

  .weak  TIMER7_Channel_IRQHandler
  .thumb_set TIMER7_Channel_IRQHandler,Default_Handler

  .weak  EXMC_IRQHandler
  .thumb_set EXMC_IRQHandler,Default_Handler

  .weak  TIMER4_IRQHandler
  .thumb_set TIMER4_IRQHandler,Default_Handler

  .weak  SPI2_IRQHandler
  .thumb_set SPI2_IRQHandler,Default_Handler

  .weak  UART3_IRQHandler
  .thumb_set UART3_IRQHandler,Default_Handler

  .weak  UART4_IRQHandler
  .thumb_set UART4_IRQHandler,Default_Handler

  .weak  TIMER5_IRQHandler
  .thumb_set TIMER5_IRQHandler,Default_Handler

  .weak  TIMER6_IRQHandler
  .thumb_set TIMER6_IRQHandler,Default_Handler

  .weak  DMA1_Channel0_IRQHandler
  .thumb_set DMA1_Channel0_IRQHandler,Default_Handler

  .weak  DMA1_Channel1_IRQHandler
  .thumb_set DMA1_Channel1_IRQHandler,Default_Handler

  .weak  DMA1_Channel2_IRQHandler
  .thumb_set DMA1_Channel2_IRQHandler,Default_Handler

  .weak  DMA1_Channel3_IRQHandler
  .thumb_set DMA1_Channel3_IRQHandler,Default_Handler

  .weak  DMA1_Channel4_IRQHandler
  .thumb_set DMA1_Channel4_IRQHandler,Default_Handler

  .weak  ENET_IRQHandler
  .thumb_set ENET_IRQHandler,Default_Handler

  .weak  ENET_WKUP_IRQHandler
  .thumb_set ENET_WKUP_IRQHandler ,Default_Handler

  .weak  CAN1_TX_IRQHandler
  .thumb_set CAN1_TX_IRQHandler ,Default_Handler

  .weak  CAN1_RX0_IRQHandler
  .thumb_set CAN1_RX0_IRQHandler ,Default_Handler

  .weak  CAN1_RX1_IRQHandler
  .thumb_set CAN1_RX1_IRQHandler ,Default_Handler

  .weak  CAN1_EWMC_IRQHandler
  .thumb_set CAN1_EWMC_IRQHandler ,Default_Handler

  .weak  USBFS_IRQHandler
  .thumb_set USBFS_IRQHandler ,Default_Handler
