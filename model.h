/*
* endpoint.h
*
*  Created on: Sep 5, 2019
*      Author: turo
*/

#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <stdint.h>
#include <string.h>

#include "lwip/ip_addr.h"
#include "./color.h"
#include "./driver.h"

namespace lightguy {

class Model {
public:
    static constexpr size_t stripN = 2;
    static constexpr size_t analogN = 2;
    static constexpr size_t universeN = 6;
    static constexpr size_t analogCompN = 5;

    struct AnalogConfig {
        uint32_t type;
        struct Component {
            uint16_t universe;
            uint16_t offset;
            uint16_t value;
        } components[analogCompN];
    };
    
    struct StripConfig {
        uint32_t type;
        rgb8 color;
        uint16_t len;
        uint16_t universe[universeN];
    };

    enum OutputConfig {
        OUTPUT_CONFIG_DUAL_STRIP, 	    // channel0: strip      channel1: strip
        OUTPUT_CONFIG_RGB_STRIP, 	    // channel0: strip 	    channel1: rgb
        OUTPUT_CONFIG_RGB_DUAL_STRIP, 	// channel0: single	    channel1: single     channel2: rgb
        OUTPUT_CONFIG_RGBW_STRIP, 	    // channel0: single	    channel1: rgbw
        OUTPUT_CONFIG_RGB_RGB, 	        // channel0: rgb 	    channel1: rgb
    };

    static Model &instance();
    
    bool burstMode() const { return burst_mode; }

    float globPWMLimit() const { return glob_pwmlimit; }
    void setGlobPWMLimit(float value) { glob_pwmlimit = value; }
    
    float globIllum() const { return glob_illum; }
    void setGlobIllum(float value) { glob_illum = value; }

    float globCompLimit() const { return glob_comp_lim; }
    void setGlobCompLimit(float value) { glob_comp_lim = value; }

    bool dhcpEnabled() const { return ip_dhcp; }
    void setDhcpEnabled(bool state) { ip_dhcp = state; }
    
    bool broadcastEnabled() const { return receive_broadcast; }
    void setBroadcastEnabled(bool state) { receive_broadcast = state; }

    const ip_addr_t *ip4Address() const { return &ip4_address; }
    const ip_addr_t *ip4Netmask() const { return &ip4_netmask; }
    const ip_addr_t *ip4Gateway() const { return &ip4_gateway; }
    
    const StripConfig &stripConfig(size_t index) const { return strip_config[index]; }
    void setStripConfig(size_t index, StripConfig config) { strip_config[index] = config; }
    const AnalogConfig &analogConfig(size_t index) const { return analog_config[index]; }
    void setAnalogConfig(size_t index, AnalogConfig config) { analog_config[index] = config; }

    OutputConfig outputConfig() const { return output_config; }
    void setOutputConfig(OutputConfig outputConfig);
    
    uint16_t universeStrip(int32_t strip, int32_t dmx512Index) const { 
        strip %= stripN;
        dmx512Index %= universeN;
        return strip_config[strip].universe[dmx512Index]; 
    }

private:
    Model() {};

    void readFlash();
    void writeFlash();

    bool initialized = false;
    void init();

    bool ip_dhcp;
    bool receive_broadcast;
    
    ip_addr_t ip4_address;
    ip_addr_t ip4_netmask;
    ip_addr_t ip4_gateway;
    
    rgb8 rgb_start_colors[2];
    rgb8 strip_start_colors[2];

    OutputConfig output_config;

    bool burst_mode;
    float glob_pwmlimit;
    float glob_illum;
    float glob_comp_lim;
    
    StripConfig strip_config[stripN];
    AnalogConfig analog_config[analogN];
};

} /* namespace model */

#endif /* ENDPOINT_H_ */
