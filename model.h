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

	void load();
    void save();
    void reset();
    void apply();
    
    bool burstMode() const { return burst_mode; }

    float globPWMLimit() const { return glob_pwmlimit; }
    void setGlobPWMLimit(float value) { glob_pwmlimit = value; }
    
    float globIllum() const { return glob_illum; }
    void setGlobIllum(float value) { glob_illum = value; }

    float globCompLimit() const { return glob_comp_lim; }
    void setGlobCompLimit(float value) { glob_comp_lim = value; }

    bool dhcpEnabled() const { return dhcp; }
    void setDhcpEnabled(bool state) { dhcp = state; }
    
    bool broadcastEnabled() const { return receive_broadcast; }
    void setBroadcastEnabled(bool state) { receive_broadcast = state; }

    ip_addr_t *ip4Address() { return &ip4_address; }
    ip_addr_t *ip4Netmask() { return &ip4_netmask; }
    ip_addr_t *ip4Gateway() { return &ip4_gateway; }

    RGBColorSpace &rgbColorSpace() { return rgbSpace; }
    
    StripConfig &stripConfig(size_t index) { return strip_config[index]; }
    AnalogConfig &analogConfig(size_t index) { return analog_config[index]; }

    OutputConfig outputConfig() const { return output_config; }
    void setOutputConfig(OutputConfig outputConfig);
    
    uint16_t universeStrip(int32_t strip, int32_t dmx512Index) const { 
        strip %= stripN;
        dmx512Index %= universeN;
        return strip_config[strip].universe[dmx512Index]; 
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

    float glob_pwmlimit;
    float glob_illum;
    float glob_comp_lim;
    
    StripConfig strip_config[stripN];
    AnalogConfig analog_config[analogN];
    
    RGBColorSpace rgbSpace;

    bool initialized = false;
};

} /* namespace model */

#endif /* ENDPOINT_H_ */
