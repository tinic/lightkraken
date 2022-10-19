extern "C" {
#include "lwip/sys.h"
}

#include "./systick.h"

extern "C" {
u32_t sys_now(void) {
    return lightkraken::Systick::instance().systemTime();
}

__attribute__((weak)) int _close(int) {
    return 0;
}

__attribute__((weak)) long _lseek(int, long, int) {
    return 0;
}

__attribute__((weak)) int _read(int, void *, unsigned) {
    return 0;
}

__attribute__((weak)) int _write(int, char *, int) {
    return 0;
}

__attribute__((weak)) int _getpid(void) {
    return 0;
}

__attribute__((weak)) int _fstat(int, struct _stat *) {
    return 0;
}

__attribute__((weak)) int _isatty(int) {
    return 0;
}

__attribute__((weak)) int _kill(int , int) {
    return 0;
}

}
