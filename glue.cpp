extern "C" {
#include "lwip/sys.h"
}

#include "./systick.h"

extern "C" {
u32_t sys_now(void) {
    return lightkraken::Systick::instance().systemTime();
}
}
