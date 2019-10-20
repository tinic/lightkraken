
#include <stdint.h>

#include "gd32f10x.h"
#include "cmsis_gcc.h"

extern "C" {
#include "lwip/apps/httpd.h"
};

#if LWIP_HTTPD_SUPPORT_REST

err_t httpd_rest_begin(void *, rest_method_t, const char *, const char *, u16_t, int, u8_t *) {
    return ERR_OK;
}

err_t httpd_rest_finished(void *, char **, u16_t *) {
    return ERR_OK;
}

err_t httpd_rest_receive_data(void *, struct pbuf *) {
    return ERR_OK;
}

#endif  // #if LWIP_HTTPD_SUPPORT_REST
