extern "C" {
#include "lwip/sys.h"
}

#include "./hardware.h"

extern "C" {
u32_t sys_now(void) {
    return system_time();
}
}
