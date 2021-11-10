/*
Copyright 2019 Tinic Uro

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <stdint.h>
#include <string.h>

#include "lwip/ip_addr.h"
#include "./color.h"
#include "./driver.h"

namespace lightkraken {

struct Model {
private:
    uint32_t model_version;

public:
    static constexpr uint32_t currentModelVersion = 0x1ed50002;

    static constexpr size_t stripN = 2;
    static constexpr size_t analogN = 2;
    static constexpr size_t universeN = 6;
    static constexpr size_t analogCompN = 5;
    static constexpr size_t maxUniverses = stripN * universeN + analogN * analogCompN;

    struct AnalogConfig {
        uint32_t output_type;
        uint32_t input_type;
        float pwm_limit;
        RGBColorSpace rgbSpace;
        struct Component {
            struct {
                uint16_t universe;
                uint16_t channel;
            } artnet;
            struct {
                uint16_t universe;
                uint16_t channel;
            } e131;
            uint16_t value;
        } components[analogCompN];
    };
    
    struct StripConfig {
        uint32_t output_type;
        uint32_t input_type;
        uint32_t startup_mode;
        float comp_limit;
        float glob_illum;
        rgb8 color;
        RGBColorSpace rgbSpace;
        uint16_t len;
        uint16_t artnet[universeN];
        uint16_t e131[universeN];
    };
    
    enum OutputConfig {
        OUTPUT_CONFIG_DUAL_STRIP, 	    // channel0: strip      channel1: strip
        OUTPUT_CONFIG_RGB_STRIP, 	    // channel0: strip 	    channel1: rgb
        OUTPUT_CONFIG_RGB_DUAL_STRIP, 	// channel0: single	    channel1: single     channel2: rgb
        OUTPUT_CONFIG_RGBW_STRIP, 	    // channel0: single	    channel1: rgbw
        OUTPUT_CONFIG_RGB_RGB, 	        // channel0: rgb 	    channel1: rgb
        OUTPUT_CONFIG_RGBWWW, 	        // channel0: rgbwww 	
    };

    static Model &instance();

    void load();
    void save();
    void reset();
    void apply();
    
    bool burstMode() const { return burst_mode; }

    bool dhcpEnabled() const { return dhcp; }
    void setDhcpEnabled(bool state) { dhcp = state; }
    
    const char *tag() const { return tag_str; }
    void setTag(const char *str);
    
    bool broadcastEnabled() const { return receive_broadcast; }
    void setBroadcastEnabled(bool state) { receive_broadcast = state; }

    ip_addr_t *ip4Address() { return &ip4_address; }
    ip_addr_t *ip4Netmask() { return &ip4_netmask; }
    ip_addr_t *ip4Gateway() { return &ip4_gateway; }
    
    StripConfig &stripConfig(size_t index) { return strip_config[index]; }
    AnalogConfig &analogConfig(size_t index) { return analog_config[index]; }

    OutputConfig outputConfig() const { return output_config; }
    void setOutputConfig(OutputConfig outputConfig);

    uint16_t artnetStrip(int32_t strip, int32_t dmx512Index) const { 
        strip %= stripN;
        dmx512Index %= universeN;
        return strip_config[strip].artnet[dmx512Index]; 
    }

    uint16_t e131Strip(int32_t strip, int32_t dmx512Index) const { 
        strip %= stripN;
        dmx512Index %= universeN;
        return strip_config[strip].e131[dmx512Index]; 
    }

private:
    Model() {};

    void defaults();
    void readFlash();
    void writeFlash();

    void init();

    bool dhcp;
    bool receive_broadcast;
    
    ip_addr_t ip4_address;
    ip_addr_t ip4_netmask;
    ip_addr_t ip4_gateway;
    
    OutputConfig output_config;

    bool burst_mode;
    
    StripConfig strip_config[stripN];
    AnalogConfig analog_config[analogN];
    
    char tag_str[256];

    bool initialized = false;
};

} /* namespace model */

#endif /* ENDPOINT_H_ */
