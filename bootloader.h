#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#define USER_FLASH_FIRST_PAGE_ADDRESS   ((uint32_t)0x08040000U)

#define FMC_PAGE_SIZE           		((uint16_t)0x400U)
#define FMC_WRITE_START_ADDR    		((uint32_t)0x08008000U)
#define FMC_WRITE_END_ADDR      		((uint32_t)0x08040000U)

#endif  // #ifndef BOOTLOADER_H_
