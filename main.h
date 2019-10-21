#ifndef MAIN_H_
#define MAIN_H_

#ifndef BOOTLOADER
#define DEBUG_PRINTF(x) do { printf x; } while (0)
#else  // #ifndef BOOTLOADER
#define DEBUG_PRINTF(x)
#endif  // #ifndef BOOTLOADER

#endif  // #ifndef MAIN_H_
