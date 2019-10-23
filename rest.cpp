#include <string.h>
#include <stdint.h>

#include "gd32f10x.h"
#include "cmsis_gcc.h"

extern "C" {
#include "lwip/apps/httpd.h"
#include "lwip/ip4_addr.h"
#include "./mjson.h"
};

#include "./model.h"
#include "./netconf.h"
#include "./systick.h"

const int32_t build_number = 
#include "./build_number.h"
;

#if LWIP_HTTPD_SUPPORT_REST

#define CRLF "\r\n"

using namespace lightguy;

static char response_buf[2048];

enum RestMethod {
    MethodNone,
    MethodGetStatus,
    MethodGetSettings,
    MethodSetSettings,
};

static RestMethod rest_method = MethodNone;

err_t httpd_rest_begin(void *, rest_method_t method, const char *url, const char *, u16_t, int, u8_t *) {
    switch(method) {
        case REST_METHOD_GET: {
            if (strcmp(url, "/status") == 0) {
                rest_method = MethodGetStatus;
                return ERR_REST_ACCEPT;
            } else if (strcmp(url, "/settings") == 0) {
                rest_method = MethodGetSettings;
                return ERR_REST_ACCEPT;
            }
        } break;
        case REST_METHOD_POST: {
            if (strcmp(url, "/settings") == 0) {
                rest_method = MethodGetSettings;
                return ERR_REST_ACCEPT;
            }
        } break;
        case REST_METHOD_PUT: {
        } break;
        case REST_METHOD_PATCH: {
        } break;
        case REST_METHOD_DELETE: {
        } break;
        case REST_METHOD_NONE: {
        } break;
    }
    rest_method = MethodNone;
    return ERR_REST_DISPATCH;
}

static char *addHeader(char *buf) {
    return buf + sprintf(buf, "HTTP/1.0 200 OK" CRLF 
                              "Content-Type: application/json; charset=utf-8" CRLF 
                              "X-Content-Type-Options: nosniff" CRLF
                              "Vary: Origin, Accept-Encoding" CRLF
                              "Content-Length: @@@@@@@@@@@" CRLF
                              "Access-Control-Allow-Origin: *" CRLF 
                              "Cache-Control: no-cache" CRLF
                              CRLF);
}

static void patchContentLength(char *buf, int32_t contentLength) {
    char *contentLengthPtr = strstr(buf, "@@@@@@@@@@@");
    if (!contentLengthPtr) {
        return;
    }
    char numberStr[12];
    memset(numberStr, 0, sizeof(numberStr));
    sprintf(numberStr, "%d", int(contentLength));
    for (size_t c = sizeof(numberStr)-1; c>0; c--) {
        if (numberStr[c] == 0) numberStr[c] = ' ';
    }
    strncpy(contentLengthPtr, numberStr, 11);
}

err_t httpd_rest_finished(void *, const char **data, u16_t *dataLen) {
    switch(rest_method) {
        case MethodGetStatus: {
            char *contentBegin = addHeader(response_buf);
            char *buf = contentBegin;
            buf += sprintf(buf, "{");
            buf += sprintf(buf, "\"ipv4address\":\"%d.%d.%d.%d\",", 
                ip4_addr1(&NetConf::instance().netInterface()->ip_addr),
                ip4_addr2(&NetConf::instance().netInterface()->ip_addr),
                ip4_addr3(&NetConf::instance().netInterface()->ip_addr),
                ip4_addr4(&NetConf::instance().netInterface()->ip_addr)
            );
            buf += sprintf(buf, "\"ipv4netmask\":\"%d.%d.%d.%d\",", 
                ip4_addr1(&NetConf::instance().netInterface()->netmask),
                ip4_addr2(&NetConf::instance().netInterface()->netmask),
                ip4_addr3(&NetConf::instance().netInterface()->netmask),
                ip4_addr4(&NetConf::instance().netInterface()->netmask)
            );
            buf += sprintf(buf, "\"ipv4gateway\":\"%d.%d.%d.%d\",", 
                ip4_addr1(&NetConf::instance().netInterface()->gw),
                ip4_addr2(&NetConf::instance().netInterface()->gw),
                ip4_addr3(&NetConf::instance().netInterface()->gw),
                ip4_addr4(&NetConf::instance().netInterface()->gw)
            );
            buf += sprintf(buf, "\"systemtime\":%d,", int(Systick::instance().systemTime())); 
            buf += sprintf(buf, "\"buildnumber\":%d", int(build_number)); 
            buf += sprintf(buf, "}");
            patchContentLength(response_buf, buf - contentBegin);
            *data = response_buf;
            *dataLen = strlen(response_buf);
            return ERR_REST_200_OK;
        } break;
        case MethodGetSettings: {
            char *contentBegin = addHeader(response_buf);
            char *buf = contentBegin;
            buf += sprintf(buf, "{");
            buf += sprintf(buf, "\"dhcp\":%s,",Model::instance().dhcpEnabled()?"true":"false"); 
            buf += sprintf(buf, "\"ipv4address\":\"%d.%d.%d.%d\",", 
                ip4_addr1(Model::instance().ip4Address()),
                ip4_addr2(Model::instance().ip4Address()),
                ip4_addr3(Model::instance().ip4Address()),
                ip4_addr4(Model::instance().ip4Address())
            );
            buf += sprintf(buf, "\"ipv4netmask\":\"%d.%d.%d.%d\",", 
                ip4_addr1(Model::instance().ip4Netmask()),
                ip4_addr2(Model::instance().ip4Netmask()),
                ip4_addr3(Model::instance().ip4Netmask()),
                ip4_addr4(Model::instance().ip4Netmask())
            );
            buf += sprintf(buf, "\"ipv4gateway\":\"%d.%d.%d.%d\",", 
                ip4_addr1(Model::instance().ip4Gateway()),
                ip4_addr2(Model::instance().ip4Gateway()),
                ip4_addr3(Model::instance().ip4Gateway()),
                ip4_addr4(Model::instance().ip4Gateway())
            );
            buf += sprintf(buf, "\"outputmode\":%d,",Model::instance().outputConfig()); 
            buf += sprintf(buf, "\"globpwmlimit\":%d,",int(Model::instance().globPWMLimit()*65536)); 
            buf += sprintf(buf, "\"globcomplimit\":%d,",int(Model::instance().globCompLimit()*65536)); 
            buf += sprintf(buf, "\"globillum\":%d,",int(Model::instance().globIllum()*65536)); 
            buf += sprintf(buf, "\"rgbuniverses\":["); 
            for (size_t c=0; c<Model::channelN; c++) {
                buf += sprintf(buf, "{\"universe\":%d,\"offset\":%d}%c", Model::instance().analogRGBMap(c).universe, 
                                                Model::instance().analogRGBMap(c).offset,
                                                (c==Model::channelN-1)?' ':','
                              ); 
            }
            buf += sprintf(buf, "],");
            buf += sprintf(buf, "\"stripuniverses\":["); 
            for (size_t c=0; c<Model::stripN; c++) {
                buf += sprintf(buf, "[");
                for (size_t d=0; d<Model::universeN; d++) {
                    buf += sprintf(buf, "%d%c", Model::instance().universeStrip(c,d),
                                                (d==Model::universeN-1)?' ':','); 
                }
                buf += sprintf(buf, "]%c",(c==Model::stripN-1)?' ':',');
            }
            buf += sprintf(buf, "]");
            buf += sprintf(buf, "}");
            patchContentLength(response_buf, buf - contentBegin);
            *data = response_buf;
            *dataLen = strlen(response_buf);
            return ERR_REST_200_OK;
        } break;
        case MethodSetSettings: {
        } break;
        case MethodNone: {
        } break;
    }
    return ERR_ARG;
}

err_t httpd_rest_receive_data(void *, struct pbuf *p) {
    pbuf_free(p);
    return ERR_OK;
}

#endif  // #if LWIP_HTTPD_SUPPORT_REST
