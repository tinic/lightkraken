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

namespace lightguy {

class Model {
public:
    static constexpr size_t stripN = 2;
    static constexpr size_t universeN = 6;
    static constexpr size_t channelN = 6;

    struct AnalogRGBUniverseEntry {
        uint16_t universe;
        uint8_t offset;
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
    float globIllum() const { return glob_illum; }
    float globCompLimit() const { return glob_comp_lim; }

    bool dhcpEnabled() const { return ip_dhcp; }

    const ip_addr_t *ip4Address() const { return &ip4_address; }
    const ip_addr_t *ip4Netmask() const { return &ip4_netmask; }
    const ip_addr_t *ip4Gateway() const { return &ip4_gateway; };

    OutputConfig outputConfig() const { return output_config; }
    void setOutputConfig(OutputConfig outputConfig);
    
    const AnalogRGBUniverseEntry &analogRGBMap(int32_t channel) const { 
        return rgbMap[channel];
    }

    uint16_t universeStrip(int32_t strip, int32_t dmx512Index) const { 
        strip %= stripN;
        dmx512Index %= universeN;
        return uniStp[strip][dmx512Index]; 
    }

private:
    Model() {};

    void readFlash();
    void writeFlash();

    bool initialized = false;
    void init();

    bool ip_dhcp;
    
    ip_addr_t ip4_address;
    ip_addr_t ip4_netmask;
    ip_addr_t ip4_gateway;

    OutputConfig output_config;
    bool burst_mode;
    float glob_pwmlimit;
    float glob_illum;
    float glob_comp_lim;
    uint32_t strip_type[stripN];
    uint32_t strip_len[stripN];
    uint16_t uniStp[stripN][universeN];

    AnalogRGBUniverseEntry rgbMap[channelN];
};

} /* namespace model */

#endif /* ENDPOINT_H_ */
