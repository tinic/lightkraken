/*
* endpoint.h
*
*  Created on: Sep 5, 2019
*      Author: turo
*/

#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include <stdint.h>
#include <memory.h>

#include "lwip/ip_addr.h"

namespace lightguy {

class Model {
public:
    static constexpr size_t stripN = 2;
    static constexpr size_t universeN = 4;
    static constexpr size_t channelN = 4;

    enum OutputConfig {
        OUTPUT_CONFIG_DUAL_STRIP, 	// channel0: strip      channel1: strip
        OUTPUT_CONFIG_RGB_STRIP, 	// channel0: rgb 	    channel1: strip
        OUTPUT_CONFIG_RGBW_STRIP, 	// channel0: rgbw	    channel1: single data line strip
        OUTPUT_CONFIG_RGBW,  		// channel0: rgbw 	    channel1: n/a
        OUTPUT_CONFIG_RGBWW,  		// channel0: rgbww 	    channel1: n/a
        OUTPUT_CONFIG_RGB_RGB, 	    // channel0: rgb 	    channel1: rgb
    };

    static Model &instance();

    bool burstMode() const { return burst_mode; }
    
    float globPWMLimit() const { return glob_pwmlimit; }

    uint8_t globIllum() const { return glob_illum; }
    uint8_t globCompLimit() const { return glob_comp_lim; }

    bool dhcpEnabled() const { return ip_dhcp; }

    const ip_addr_t *ip4Address() const { return &ip4_address; }
    const ip_addr_t *ip4Netmask() const { return &ip4_netmask; }
    const ip_addr_t *ip4Gateway() const { return &ip4_gateway; };

    OutputConfig outputConfig() const { return output_config; }
    void setOutputConfig(OutputConfig outputConfig);
    
    uint16_t universeOffset(int32_t channel, size_t &uniOffset) const { 
        channel %= 6;
        uniOffset = uniOff[channel].offset; 
        return uniOff[channel].universe;
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
    uint8_t glob_illum;
    uint8_t glob_comp_lim;
    uint32_t strip_type[stripN];
    uint32_t strip_len[stripN];
    uint16_t uniStp[stripN][universeN];

    struct UniverseOffsetMapping {
        uint16_t universe;
        uint8_t offset;
    };
    
    UniverseOffsetMapping uniOff[channelN];
};

} /* namespace model */

#endif /* ENDPOINT_H_ */
