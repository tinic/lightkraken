#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <algorithm>

#include "gd32f10x.h"
#include "cmsis_gcc.h"

extern "C" {
#include "lwip/apps/httpd.h"
#include "lwip/ip4_addr.h"
#include "./mjson.h"
#include "./ftoa.h"
#include "./jsonsl.h"
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

namespace lightkraken {

#if 0
class HTTPPost {
public:
    static HTTPPost &instance();
    
    void begin() {
        buf_ptr = post_buf;
        memset(post_buf, 0, sizeof(post_buf));
    }
    
    void pushData(void *data, size_t len) {
        size_t buf_max_len = sizeof(post_buf) - 1;
        size_t buf_cur_len = buf_ptr - post_buf;
        len = std::min(buf_cur_len + len, buf_max_len) - buf_cur_len;
        if (len > 0) {
            memcpy(buf_ptr, data, len);
            buf_ptr += len;
        }
    }
    
    void end() {
        char buf[64];
        int ival = 0;
        double dval = 0;
        int res = 0;
        size_t post_len = strlen(post_buf);
        res = mjson_get_bool(post_buf, post_len, "$.dhcp", &ival);
        if (res > 0) {
            Model::instance().setDhcpEnabled(ival ? true : false);
        }
        res =  mjson_get_bool(post_buf, post_len, "$.broadcast", &ival);
        if (res > 0) {
            Model::instance().setBroadcastEnabled(ival ? true : false);
        }
        res = mjson_get_string(post_buf, post_len, "$.ipv4address", buf, sizeof(buf));
        if (res > 0) {
            int ipbits[4];
            sscanf(buf, "%d.%d.%d.%d", &ipbits[0], &ipbits[1], &ipbits[2], &ipbits[3]);
            IP4_ADDR(Model::instance().ip4Address(), ipbits[0], ipbits[1], ipbits[2], ipbits[3]);
        }
        res = mjson_get_string(post_buf, post_len, "$.ipv4netmask", buf, sizeof(buf));
        if (res > 0) {
            int ipbits[4];
            sscanf(buf, "%d.%d.%d.%d", &ipbits[3], &ipbits[2], &ipbits[1], &ipbits[0]);
            IP4_ADDR(Model::instance().ip4Netmask(), ipbits[0], ipbits[1], ipbits[2], ipbits[3]);
        }
        res = mjson_get_string(post_buf, post_len, "$.ipv4gateway", buf, sizeof(buf));
        if (res > 0) {
            int ipbits[4];
            sscanf(buf, "%d.%d.%d.%d", &ipbits[3], &ipbits[2], &ipbits[1], &ipbits[0]);
            IP4_ADDR(Model::instance().ip4Gateway(), ipbits[0], ipbits[1], ipbits[2], ipbits[3]);
        }
        res = mjson_get_number(post_buf, post_len, "$.outputmode", &dval);
        if (res > 0) {
            Model::instance().setOutputConfig(Model::OutputConfig(int(dval)));
        }
        res = mjson_get_number(post_buf, post_len, "$.globpwmlimit", &dval);
        if (res > 0) {
            Model::instance().setGlobPWMLimit(float(dval));
        }
        res = mjson_get_number(post_buf, post_len, "$.globcomplimit", &dval);
        if (res > 0) {
            Model::instance().setGlobCompLimit(float(dval));
        }
        res = mjson_get_number(post_buf, post_len, "$.globillum", &dval);
        if (res > 0) {
            Model::instance().setGlobIllum(float(dval));
        }
        
        for (int c=0; c<int(Model::analogN); c++) {
            Model::AnalogConfig config = Model::instance().analogConfig(c);
            char ss[32];
            sprintf(ss, "$.rgbconfig[%d].type", c);
            res = mjson_get_number(post_buf, post_len, ss, &dval);
            if (res > 0) {
                config.type = int(dval);
            }
            for (int d=0; d<int(Model::analogCompN); d++) {
                sprintf(ss, "$.rgbconfig[%d].components[%d].universe", c, d);
                res = mjson_get_number(post_buf, post_len, ss, &dval);
                if (res > 0) {
                    config.components[d].universe = int(dval);
                }
                sprintf(ss, "$.rgbconfig[%d].components[%d].offset", c, d);
                res = mjson_get_number(post_buf, post_len, ss, &dval);
                if (res > 0) {
                    config.components[d].offset = int(dval);
                }
                sprintf(ss, "$.rgbconfig[%d].components[%d].value", c, d);
                res = mjson_get_number(post_buf, post_len, ss, &dval);
                if (res > 0) {
                    config.components[d].value = int(dval);
                }
            }
            Model::instance().setAnalogConfig(c, config);
        }

        for (int c=0; c<int(Model::stripN); c++) {
            Model::StripConfig config = Model::instance().stripConfig(c);
            char ss[32];
            sprintf(ss, "$.stripconfig[%d].type", c);
            res = mjson_get_number(post_buf, post_len, ss, &dval);
            if (res > 0) {
                config.type = int(dval);
            }
            sprintf(ss, "$.stripconfig[%d].length", c);
            res = mjson_get_number(post_buf, post_len, ss, &dval);
            if (res > 0) {
                config.len = int(dval);
            }
            sprintf(ss, "$.stripconfig[%d].color", c);
            res = mjson_get_string(post_buf, post_len, ss, buf, sizeof(buf));
            if (res > 0) {
                config.color.rgbx = strtol(buf, NULL, 16);
            }
            for (int d=0; d<int(Model::universeN); d++) {
                sprintf(ss, "$.stripconfig[%d].universes[%d].universe", c, d);
                res = mjson_get_number(post_buf, post_len, ss, &dval);
                if (res > 0) {
                    config.universe[d] = int(dval);
                }
            }
            Model::instance().setStripConfig(c, config);
        }
    }
    
private:
    bool initialized = false;
    void init();
    char *buf_ptr;
    char post_buf[1536];
};

HTTPPost &HTTPPost::instance() {
    static HTTPPost httpPost;
    if (!httpPost.initialized) {
        httpPost.initialized = true;
        httpPost.init();
    }
    return httpPost;
}

void HTTPPost::init() {
}
#endif 

class HTTPResponse {
public:
    static HTTPResponse &instance();
    
    void beginOKResponse() {
        buf_ptr = response_buf;
        buf_ptr += sprintf(buf_ptr, "HTTP/1.0 200 OK" CRLF
                                    "Access-Control-Allow-Origin: *" CRLF);
        responseType = OKResponse;
    }

    void beginJSONResponse() {
        buf_ptr = response_buf;
        buf_ptr += sprintf(buf_ptr, "HTTP/1.0 200 OK" CRLF 
                                    "Access-Control-Allow-Origin: *" CRLF 
                                    "Content-Type: application/json; charset=utf-8" CRLF 
                                    "X-Content-Type-Options: nosniff" CRLF
                                    "Vary: Origin, Accept-Encoding" CRLF
                                    "Content-Length: @@@@@@@@@@@" CRLF
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
        char str[32];
        ftoa(str, Model::instance().globPWMLimit(), NULL);
        buf_ptr += sprintf(buf_ptr, "\"globpwmlimit\":%s",str); 
    }

    void addCompLimit() {
        handleDelimiter();
        char str[32];
        ftoa(str, Model::instance().globCompLimit(), NULL);
        buf_ptr += sprintf(buf_ptr, "\"globcomplimit\":%s",str); 
    }

    void addIllum() {
        handleDelimiter();
        char str[32];
        ftoa(str, Model::instance().globIllum(), NULL);
        buf_ptr += sprintf(buf_ptr, "\"globillum\":%s",str); 
    }

    void addAnalogConfig() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"rgbconfig\":["); 
        for (size_t c=0; c<Model::analogN; c++) {
            const Model::AnalogConfig &a = Model::instance().analogConfig(c);
            buf_ptr += sprintf(buf_ptr, "{");
            buf_ptr += sprintf(buf_ptr, "\"type\":%d,",int(a.type)); 
            buf_ptr += sprintf(buf_ptr, "\"components\" : [");
            for (size_t d=0; d<Model::analogCompN; d++) {
                buf_ptr += sprintf(buf_ptr, "{");
                buf_ptr += sprintf(buf_ptr, "\"universe\":%d,",int(a.components[d].universe)); 
                buf_ptr += sprintf(buf_ptr, "\"offset\":%d,",int(a.components[d].offset)); 
                buf_ptr += sprintf(buf_ptr, "\"value\":%d",int(a.components[d].value)); 
                buf_ptr += sprintf(buf_ptr, "}%c", (d==Model::analogCompN-1)?' ':','); 
            }
            buf_ptr += sprintf(buf_ptr, "]");
            buf_ptr += sprintf(buf_ptr, "}%c", (c==Model::analogN-1)?' ':',');
        }

        buf_ptr += sprintf(buf_ptr, "]");
    }

    void addStripConfig() {
        handleDelimiter();
        buf_ptr += sprintf(buf_ptr, "\"stripconfig\":["); 
        for (size_t c=0; c<Model::stripN; c++) {
            const Model::StripConfig &s = Model::instance().stripConfig(c);
            buf_ptr += sprintf(buf_ptr, "{");
            buf_ptr += sprintf(buf_ptr, "\"type\":%d,",int(s.type)); 
            buf_ptr += sprintf(buf_ptr, "\"length\":%d,",int(s.len)); 
            buf_ptr += sprintf(buf_ptr, "\"color\":\"0x%08x\",",(unsigned int)s.color.hex()); 
            buf_ptr += sprintf(buf_ptr, "\"universes\" : [");
            for (size_t d=0; d<Model::universeN; d++) {
                buf_ptr += sprintf(buf_ptr, "{");
                buf_ptr += sprintf(buf_ptr, "\"universe\":%d",int(s.universe[d])); 
                buf_ptr += sprintf(buf_ptr, "}%c", (d==Model::universeN-1)?' ':','); 
            }
            buf_ptr += sprintf(buf_ptr, "]");
            buf_ptr += sprintf(buf_ptr, "}%c", (c==Model::stripN-1)?' ':',');
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

class ConnectionManager {
public:
	enum RestMethod {
		MethodNone,
		MethodGetStatus,
		MethodGetSettings,
		MethodPostSettings,
		MethodPostBootLoader,
	};

	constexpr static size_t maxConnections = MEMP_NUM_TCP_PCB;
	constexpr static size_t jsonslLevels = 4;
	constexpr static size_t jsonslSize = sizeof(struct jsonsl_st) + (jsonslLevels - 1) * sizeof (struct jsonsl_state_st);

    static ConnectionManager &instance();

	void init() {
		memset(connections, 0, sizeof(connections));
	}

	struct ConnectionInfo {
		public:
			void *handle;
			RestMethod method;
			jsonsl_st *initJsonParser() {
				struct jsonsl_st *jsn = reinterpret_cast<struct jsonsl_st *>(&jsonsl[0]);
				jsn->levels_max = (unsigned int)jsonslLevels;
				jsn->max_callback_level = UINT_MAX;
				jsonsl_reset(jsn);
				for (size_t ii = 0; ii < jsn->levels_max; ii++) {
					jsn->stack[ii].level = ii;
				}
				return jsn;
			}

			jsonsl_st *jsonParser() {
				return reinterpret_cast<struct jsonsl_st *>(&jsonsl[0]);
			}
			
		private:
			uint8_t jsonsl[jsonslSize];
	};
	
	ConnectionInfo *begin(void *handle) {
		for (size_t c = 0; c < maxConnections; c++) {
			if (connections[c].handle == NULL) {
				connections[c].handle = handle;
				return &connections[c];
			}
		}
		return NULL;
	}
	
	ConnectionInfo *get(void *handle) {
		for (size_t c = 0; c < maxConnections; c++) {
			if (connections[c].handle == handle) {
				return &connections[c];
			}
		}
		return NULL;
	}
	
	void end(void *handle) {
		for (size_t c = 0; c < maxConnections; c++) {
			if (connections[c].handle == handle) {
				connections[c].handle = NULL;
				break;
			}
		}
	}

private:
    bool initialized = false;
	ConnectionInfo connections[maxConnections];
};

ConnectionManager &ConnectionManager::instance() {
    static ConnectionManager connectionManager;
    if (!connectionManager.initialized) {
        connectionManager.initialized = true;
        connectionManager.init();
    }
    return connectionManager;
}

}

err_t httpd_rest_begin(void *handle, rest_method_t method, const char *url, const char *, u16_t, int, u8_t *) {
	lightkraken::ConnectionManager::ConnectionInfo *info = lightkraken::ConnectionManager::instance().begin(handle);
	if (!info) {
		return ERR_ARG;
	}
    switch(method) {
        case REST_METHOD_OPTIONS: {
        } break;
        case REST_METHOD_GET: {
            if (strcmp(url, "/status") == 0) {
                info->method = lightkraken::ConnectionManager::MethodGetStatus;
                return ERR_OK;
            } else if (strcmp(url, "/settings") == 0) {
                info->method = lightkraken::ConnectionManager::MethodGetSettings;
                return ERR_OK;
            }
        } break;
        case REST_METHOD_POST: {
            if (strcmp(url, "/settings") == 0) {
                info->method = lightkraken::ConnectionManager::MethodPostSettings;
                info->initJsonParser();
                info->jsonParser();
                return ERR_OK;
            } else if (strcmp(url, "/bootloader") == 0) {
                info->method = lightkraken::ConnectionManager::MethodPostBootLoader;
                return ERR_OK;
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
	lightkraken::ConnectionManager::instance().end(handle);
    return ERR_ARG;
}

err_t httpd_rest_receive_data(void *handle, struct pbuf *p) {
	lightkraken::ConnectionManager::ConnectionInfo *info = lightkraken::ConnectionManager::instance().get(handle);
	if (!info) {
		return ERR_ARG;
	}
    switch(info->method) {
		case lightkraken::ConnectionManager::MethodPostSettings: {
			jsonsl_feed(info->jsonParser(), (const char *)p->payload, p->len);
		} break;
        case lightkraken::ConnectionManager::MethodNone:
        case lightkraken::ConnectionManager::MethodPostBootLoader:
        case lightkraken::ConnectionManager::MethodGetSettings:
        case lightkraken::ConnectionManager::MethodGetStatus:
        break;
    }
    pbuf_free(p);
    return ERR_OK;
}

err_t httpd_rest_finished(void *handle, const char **data, u16_t *dataLen) {
	lightkraken::ConnectionManager::ConnectionInfo *info = lightkraken::ConnectionManager::instance().get(handle);
	if (!info) {
		return ERR_ARG;
	}
    lightkraken::HTTPResponse &i = lightkraken::HTTPResponse::instance();
    switch(info->method) {
        case lightkraken::ConnectionManager::MethodGetStatus: {
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
        case lightkraken::ConnectionManager::MethodGetSettings: {
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
        case lightkraken::ConnectionManager::MethodPostBootLoader: {
            lightkraken::Systick::instance().scheduleReset(4000, true);
            i.beginOKResponse();
            *data = i.finish(*dataLen);
            return ERR_OK;
        } break;
        case lightkraken::ConnectionManager::MethodPostSettings: {
            i.beginOKResponse();
            *data = i.finish(*dataLen);
            return ERR_OK;
        } break;
        case lightkraken::ConnectionManager::MethodNone: {
        } break;
    }
    return ERR_ARG;
}

#endif  // #if LWIP_HTTPD_SUPPORT_REST
