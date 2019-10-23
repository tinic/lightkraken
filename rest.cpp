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

err_t httpd_rest_finished(void *, const char **data, u16_t *dataLen) {
    switch(rest_method) {
        case MethodGetStatus: {
            char *buf = response_buf;
            buf += sprintf(buf, "{");
            buf += sprintf(buf, "\"ipv4address:\"\"%d.%d.%d.%d\",", 
                ip4_addr1(&lightguy::NetConf::instance().netInterface()->ip_addr),
                ip4_addr2(&lightguy::NetConf::instance().netInterface()->ip_addr),
                ip4_addr3(&lightguy::NetConf::instance().netInterface()->ip_addr),
                ip4_addr4(&lightguy::NetConf::instance().netInterface()->ip_addr)
            );
            buf += sprintf(buf, "\"ipv4netmask\":\"%d.%d.%d.%d\",", 
                ip4_addr1(&lightguy::NetConf::instance().netInterface()->netmask),
                ip4_addr2(&lightguy::NetConf::instance().netInterface()->netmask),
                ip4_addr3(&lightguy::NetConf::instance().netInterface()->netmask),
                ip4_addr4(&lightguy::NetConf::instance().netInterface()->netmask)
            );
            buf += sprintf(buf, "\"ipv4gateway\":\"%d.%d.%d.%d\",", 
                ip4_addr1(&lightguy::NetConf::instance().netInterface()->gw),
                ip4_addr2(&lightguy::NetConf::instance().netInterface()->gw),
                ip4_addr3(&lightguy::NetConf::instance().netInterface()->gw),
                ip4_addr4(&lightguy::NetConf::instance().netInterface()->gw)
            );
            buf += sprintf(buf, "\"systemtime\":%d,", int(lightguy::Systick::instance().systemTime())); 
            buf += sprintf(buf, "\"buildnumber\":%d", int(build_number)); 
            buf += sprintf(buf, "}");
            *data = response_buf;
            *dataLen = strlen(response_buf);
            return ERR_REST_200_OK;
        } break;
        case MethodGetSettings: {
            char *buf = response_buf;
            buf += sprintf(buf, "{");
            buf += sprintf(buf, "\"dhcp\":%s,",lightguy::Model::instance().dhcpEnabled()?"true":"false"); 
            buf += sprintf(buf, "\"ipv4address\":\"%d.%d.%d.%d\",", 
                ip4_addr1(lightguy::Model::instance().ip4Address()),
                ip4_addr2(lightguy::Model::instance().ip4Address()),
                ip4_addr3(lightguy::Model::instance().ip4Address()),
                ip4_addr4(lightguy::Model::instance().ip4Address())
            );
            buf += sprintf(buf, "\"ipv4netmask\":\"%d.%d.%d.%d\",", 
                ip4_addr1(lightguy::Model::instance().ip4Netmask()),
                ip4_addr2(lightguy::Model::instance().ip4Netmask()),
                ip4_addr3(lightguy::Model::instance().ip4Netmask()),
                ip4_addr4(lightguy::Model::instance().ip4Netmask())
            );
            buf += sprintf(buf, "\"ipv4gateway\":\"%d.%d.%d.%d\",", 
                ip4_addr1(lightguy::Model::instance().ip4Gateway()),
                ip4_addr2(lightguy::Model::instance().ip4Gateway()),
                ip4_addr3(lightguy::Model::instance().ip4Gateway()),
                ip4_addr4(lightguy::Model::instance().ip4Gateway())
            );
            buf += sprintf(buf, "\"outputmode\":%d,",lightguy::Model::instance().outputConfig()); 
            buf += sprintf(buf, "\"globpwmlimit\":%d,",int(lightguy::Model::instance().globPWMLimit()*65536)); 
            buf += sprintf(buf, "\"globcomplimit\":%d,",int(lightguy::Model::instance().globCompLimit()*65536)); 
            buf += sprintf(buf, "\"globillum\":%d,",int(lightguy::Model::instance().globIllum()*65536)); 
            buf += sprintf(buf, "\"rgbuniverses\":["); 
            for (size_t c=0; c<lightguy::Model::channelN; c++) {
                buf += sprintf(buf, "{\"universe\":%d,\"offset\":%d}%c", lightguy::Model::instance().analogRGBMap(c).universe, 
                                                lightguy::Model::instance().analogRGBMap(c).offset,
                                                (c==lightguy::Model::channelN-1)?' ':','
                              ); 
            }
            buf += sprintf(buf, "],");
            buf += sprintf(buf, "\"stripuniverses\":["); 
            for (size_t c=0; c<lightguy::Model::stripN; c++) {
                buf += sprintf(buf, "[");
                for (size_t d=0; d<lightguy::Model::universeN; d++) {
                    buf += sprintf(buf, "%d%c", lightguy::Model::instance().universeStrip(c,d),
                                                (d==lightguy::Model::universeN-1)?' ':','); 
                }
                buf += sprintf(buf, "]%c",(c==lightguy::Model::stripN-1)?' ':',');
            }
            buf += sprintf(buf, "]");
            buf += sprintf(buf, "}");
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
