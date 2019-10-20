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

    enum OutputConfig {
        OUTPUT_CONFIG_DUAL_STRIP, 	// channel0: strip 	channel1: strip
        OUTPUT_CONFIG_RGB_STRIP, 	// channel0: rgb 	channel1: strip
        OUTPUT_CONFIG_RGBW  		// channel0: rgbw 	channel1: n/a
    };

    static Model &instance();

    bool burstMode() const { return burst_mode; }

    uint8_t globIllum() const { return glob_illum; }
    uint8_t globCompLimit() const { return glob_comp_lim; }

    bool dhcpEnabled() const { return ip_dhcp; }

    const ip_addr_t *ip4Address() const { return &ip4_address; }
    const ip_addr_t *ip4Netmask() const { return &ip4_netmask; }
    const ip_addr_t *ip4Gateway() const { return &ip4_gateway; };

    OutputConfig outputConfig() const { return output_config; }
    void setOutputConfig(OutputConfig outputConfig);
    
    uint16_t universe(int32_t strip, int32_t index) const { return uni[strip][index]; }

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
    uint8_t glob_illum;
    uint8_t glob_comp_lim;
    uint32_t strip_type[stripN];
    uint16_t uni[stripN][universeN];
};

} /* namespace model */

#endif /* ENDPOINT_H_ */
