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
	enum OutputConfig {
		OUTPUT_CONFIG_DUAL_STRIP, 	// channel0: strip 	channel1: strip
		OUTPUT_CONFIG_RGB_STRIP, 	// channel0: rgb 	channel1: strip
		OUTPUT_CONFIG_RGBW  		// channel0: rgbw 	channel1: n/a
	};

	static constexpr size_t stripN = 2;
	static constexpr size_t universeN = 6;

	static Model &instance();

	const uint8_t *macAddress() const { return &mac_address[0]; }

	bool burstMode() const { return burst_mode; }

	uint8_t globIllum() const { return glob_illum; }
	uint8_t globCompLimit() const { return glob_comp_lim; }

	void setUniverseOutputData(uint16_t universe, const uint8_t *data, size_t len);
	void setOutputData(uint8_t channel, const uint8_t *data, size_t len);

	void transferNow();
  
	bool dhcpEnabled() const { return ip_dhcp; }

	const ip_addr_t *ip4Interface() const { return &ip4_interface; }
	const ip_addr_t *ip4Netmask() const { return &ip4_netmask; }
	const ip_addr_t *ip4Gateway() const { return &ip4_gateway; };

	OutputConfig outputConfig() const { return output_config; }
	void setOutputConfig(OutputConfig outputConfig);

    void updateUserLED();

private:
	Model() {};

	void readFlash();
	void writeFlash();

    void readPowerState();
	void setUserLED(uint8_t r, uint8_t g, uint8_t b);

	bool initialized = false;
	void init();

	bool ip_dhcp;
	ip_addr_t ip4_interface;
	ip_addr_t ip4_netmask;
	ip_addr_t ip4_gateway;

	OutputConfig output_config;
	bool burst_mode;
	uint8_t glob_illum;
	uint8_t glob_comp_lim;
	uint32_t strip_type[stripN];
	uint16_t striplen[stripN];
	uint16_t universe[stripN][universeN];
	uint8_t mac_address[6];

  	bool bt_state = false;
	bool tpl_state = false;
	bool tph_state = false;
	bool powergood_state = false;

};

} /* namespace model */

#endif /* ENDPOINT_H_ */
