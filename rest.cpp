#include <string.h>
#include <stdint.h>

#include "gd32f10x.h"
#include "cmsis_gcc.h"

extern "C" {
#include "lwip/apps/httpd.h"
#include "lwip/ip4_addr.h"
#include "./mjson.h"
};

#include "./color.h"
#include "./model.h"
#include "./netconf.h"
#include "./systick.h"

const int32_t build_number = 
#include "./build_number.h"
;

#if LWIP_HTTPD_SUPPORT_REST

#define CRLF "\r\n"

namespace lightguy {
    
class HTTPResponse {
public:
    static HTTPResponse &instance();
    
    void beginOKResponse() {
        buf_ptr = response_buf;
        buf_ptr += sprintf(buf_ptr, "HTTP/1.0 200 OK" CRLF);
        responseType = OKResponse;
    }

    void beginJSONResponse() {
        buf_ptr = response_buf;
        buf_ptr += sprintf(buf_ptr, "HTTP/1.0 200 OK" CRLF 
                                    "Content-Type: application/json; charset=utf-8" CRLF 
                                    "X-Content-Type-Options: nosniff" CRLF
                                    "Vary: Origin, Accept-Encoding" CRLF
                                    "Content-Length: @@@@@@@@@@@" CRLF
                                    "Access-Control-Allow-Origin: *" CRLF 
                                    "Cache-Control: no-cache" CRLF
                                    CRLF);
        content_start = buf_ptr;
        buf_ptr += sprintf(buf_ptr, "{");
        first_item = true;
        responseType = JSONResponse;
    }
    
    const char *finish(u16_t &length) {
        switch (responseType) {
        case JSONResponse: {
            buf_ptr += sprintf(buf_ptr, "}");
            
            // Patch content length
            char *contentLengthPtr = strstr(response_buf, "@@@@@@@@@@@");
            if (!contentLengthPtr) {
                return 0;
            }

            char numberStr[12];
            memset(numberStr, 0, sizeof(numberStr));
            sprintf(numberStr, "%d", int(buf_ptr - content_start));
            for (size_t c = sizeof(numberStr)-1; c>0; c--) {
                if (numberStr[c] == 0) numberStr[c] = ' ';
            }
            strncpy(contentLengthPtr, numberStr, 11);

            length = u16_t(buf_ptr - response_buf);
            return response_buf;
        } break;
        case OKResponse: {
            length = u16_t(buf_ptr - response_buf);
            return response_buf;
        } break;
        }
        return 0;
    }
    
    void handleDelimiter() {
        if (first_item) {
            first_item = false;
        } else {
            buf_ptr += sprintf(buf_ptr, ",");
        }
    }

    void addNetConfIPv4Address() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"ipv4address\":\"%d.%d.%d.%d\"", 
                ip4_addr1(&NetConf::instance().netInterface()->ip_addr),
                ip4_addr2(&NetConf::instance().netInterface()->ip_addr),
                ip4_addr3(&NetConf::instance().netInterface()->ip_addr),
                ip4_addr4(&NetConf::instance().netInterface()->ip_addr)
            );
    }

    void addNetConfIPv4Netmask() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"ipv4netmask\":\"%d.%d.%d.%d\"", 
                ip4_addr1(&NetConf::instance().netInterface()->netmask),
                ip4_addr2(&NetConf::instance().netInterface()->netmask),
                ip4_addr3(&NetConf::instance().netInterface()->netmask),
                ip4_addr4(&NetConf::instance().netInterface()->netmask)
            );
    }

    void addNetConfIPv4Gateway() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"ipv4gateway\":\"%d.%d.%d.%d\"", 
                ip4_addr1(&NetConf::instance().netInterface()->gw),
                ip4_addr2(&NetConf::instance().netInterface()->gw),
                ip4_addr3(&NetConf::instance().netInterface()->gw),
                ip4_addr4(&NetConf::instance().netInterface()->gw)
            );
    }

    void addSystemTime() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"systemtime\":%d", int(Systick::instance().systemTime())); 
    }

    void addBuildNumber() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"buildnumber\":%d", int(build_number)); 
    }

    void addHostname() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"hostname\":\"%s\"", NetConf::instance().netInterface()->hostname); 
    }

    void addMacAddress() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"macaddress\":\"%02x:%02x:%02x:%02x:%02x:%02x\"", 
                        NetConf::instance().netInterface()->hwaddr[0],
                        NetConf::instance().netInterface()->hwaddr[1],
                        NetConf::instance().netInterface()->hwaddr[2],
                        NetConf::instance().netInterface()->hwaddr[3],
                        NetConf::instance().netInterface()->hwaddr[4],
                        NetConf::instance().netInterface()->hwaddr[5]); 
    }
    
    void addDHCP() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"dhcp\":%s",Model::instance().dhcpEnabled()?"true":"false"); 
    }

    void addBroadcast() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"broadcast\":%s",Model::instance().broadcastEnabled()?"true":"false"); 
    }
    
    void addIPv4Address() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"ipv4address\":\"%d.%d.%d.%d\"", 
            ip4_addr1(Model::instance().ip4Address()),
            ip4_addr2(Model::instance().ip4Address()),
            ip4_addr3(Model::instance().ip4Address()),
            ip4_addr4(Model::instance().ip4Address())
        );
    }
    
    
    void addIPv4Netmask() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"ipv4netmask\":\"%d.%d.%d.%d\"", 
            ip4_addr1(Model::instance().ip4Netmask()),
            ip4_addr2(Model::instance().ip4Netmask()),
            ip4_addr3(Model::instance().ip4Netmask()),
            ip4_addr4(Model::instance().ip4Netmask())
        );
    }
    
    void addIPv4Gateway() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"ipv4gateway\":\"%d.%d.%d.%d\"", 
            ip4_addr1(Model::instance().ip4Gateway()),
            ip4_addr2(Model::instance().ip4Gateway()),
            ip4_addr3(Model::instance().ip4Gateway()),
            ip4_addr4(Model::instance().ip4Gateway())
        );
    }
    
    void addOutputMode() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"outputmode\":%d",Model::instance().outputConfig()); 
    }

    void addPwmLimit() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"globpwmlimit\":%d",int(Model::instance().globPWMLimit()*65536)); 
    }

    void addCompLimit() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"globcomplimit\":%d",int(Model::instance().globCompLimit()*65536)); 
    }

    void addIllum() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"globillum\":%d",int(Model::instance().globIllum()*65536)); 
    }

    void addAnalogConfig() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"rgbconfig\":["); 
        for (size_t c=0; c<Model::analogN; c++) {
            buf_ptr += sprintf(buf_ptr, "{");
            buf_ptr += sprintf(buf_ptr, "\"type\":%d,",int(Model::instance().analogConfig(c).type)); 
            buf_ptr += sprintf(buf_ptr, "\"components\" : [");
            for (size_t d=0; d<Model::analogCompN; d++) {
                buf_ptr += sprintf(buf_ptr, "{");
                buf_ptr += sprintf(buf_ptr, "\"universe\":%d,",int(Model::instance().analogConfig(c).components[d].universe)); 
                buf_ptr += sprintf(buf_ptr, "\"offset\":%d,",int(Model::instance().analogConfig(c).components[d].offset)); 
                buf_ptr += sprintf(buf_ptr, "\"value\":%d",int(Model::instance().analogConfig(c).components[d].value)); 
                buf_ptr += sprintf(buf_ptr, "}%c", (d==Model::analogCompN-1)?' ':','); 
            }
            buf_ptr += sprintf(buf_ptr, "]");
            buf_ptr += sprintf(buf_ptr, "}%c",(c==Model::analogN-1)?' ':',');
        }

        buf_ptr += sprintf(buf_ptr, "]");
    }

    void addStripConfig() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"stripconfig\":["); 
        for (size_t c=0; c<Model::stripN; c++) {
            buf_ptr += sprintf(buf_ptr, "{");
            buf_ptr += sprintf(buf_ptr, "\"type\":%d,",int(Model::instance().stripConfig(c).type)); 
            buf_ptr += sprintf(buf_ptr, "\"length\":%d,",int(Model::instance().stripConfig(c).len)); 
            buf_ptr += sprintf(buf_ptr, "\"color\":\"0x%08x\",",(unsigned int)Model::instance().stripConfig(c).color.hex()); 
            buf_ptr += sprintf(buf_ptr, "\"universes\" : [");
            for (size_t d=0; d<Model::universeN; d++) {
                buf_ptr += sprintf(buf_ptr, "%d%c", Model::instance().stripConfig(c).universe[d],
                                            (d==Model::universeN-1)?' ':','); 
            }
            buf_ptr += sprintf(buf_ptr, "]");
            buf_ptr += sprintf(buf_ptr, "}%c",(c==Model::stripN-1)?' ':',');
        }
        buf_ptr += sprintf(buf_ptr, "]");
    }
    
private:
    enum ResponseType {
        OKResponse = 0,
        JSONResponse = 1,
    } responseType = OKResponse;
    
    bool initialized = false;
    void init();
    
    bool first_item = true;
    char *buf_ptr;
    char *content_start;
    char response_buf[2048];
};

HTTPResponse &HTTPResponse::instance() {
    static HTTPResponse httpResponse;
    if (!httpResponse.initialized) {
        httpResponse.initialized = true;
        httpResponse.init();
    }
    return httpResponse;
}

void HTTPResponse::init() {
}

}

using namespace lightguy;

enum RestMethod {
    MethodNone,
    MethodGetStatus,
    MethodGetSettings,
    MethodSetSettings,
    MethodPostBootLoader,
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
            } else if (strcmp(url, "/bootloader") == 0) {
                rest_method = MethodPostBootLoader;
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

err_t httpd_rest_receive_data(void *, struct pbuf *p) {
    pbuf_free(p);
    return ERR_OK;
}

err_t httpd_rest_finished(void *, const char **data, u16_t *dataLen) {
    HTTPResponse &i = HTTPResponse::instance();
    switch(rest_method) {
        case MethodGetStatus: {
            i.beginJSONResponse();
            i.addNetConfIPv4Address();
            i.addNetConfIPv4Netmask();
            i.addNetConfIPv4Gateway();
            i.addSystemTime();
            i.addBuildNumber();
            i.addHostname();
            i.addMacAddress();
            *data = i.finish(*dataLen);
            return ERR_OK;
        } break;
        case MethodGetSettings: {
            i.beginJSONResponse();
            i.addDHCP();
            i.addBroadcast();
            i.addIPv4Address();
            i.addIPv4Netmask();
            i.addIPv4Gateway();
            i.addOutputMode();
            i.addPwmLimit();
            i.addCompLimit();
            i.addIllum();
            i.addAnalogConfig();
            i.addStripConfig();
            *data = i.finish(*dataLen);
            return ERR_OK;
        } break;
        case MethodPostBootLoader: {
            lightguy::Systick::instance().scheduleReset(4000, true);
            i.beginOKResponse();
            *data = i.finish(*dataLen);
            return ERR_OK;
        } break;
        case MethodSetSettings: {
        } break;
        case MethodNone: {
        } break;
    }
    return ERR_ARG;
}

#endif  // #if LWIP_HTTPD_SUPPORT_REST
