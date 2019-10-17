/*
 * endpoint.cpp
 *
 *  Created on: Sep 5, 2019
 *      Author: turo
 */

extern "C" {
#include "gd32f10x.h"
}

#include "./model.h"
#include "./strip.h"
#include "./driver.h"

namespace lightguy {

/*
static uint32_t readu32(const uint8_t **page) {
	uint32_t v = ((*page[0])    )|
				 ((*page[1])<< 8)|
				 ((*page[2])<<16)|
				 ((*page[3])<<24);
	*page+=4;
	return v;
}

static uint16_t readu16(const uint8_t **page) {
	uint16_t v = ((*page[0])    )|
				 ((*page[1])<< 8);
	*page+=2;
	return v;
}
*/

void Model::readFlash() {
}

/*
static void writeu32(uintptr_t *addr, uint32_t v) {
    (void)v;
	*addr += 4;
}

static void writeu16(uintptr_t *addr, uint16_t v) {
    (void)v;
	*addr += 2;
}
*/

void Model::writeFlash() {
}

void Model::init() {

	ip_dhcp = true;

	glob_illum = 0x1F;
	glob_comp_lim = 0xFF;

	output_config = OUTPUT_CONFIG_DUAL_STRIP;

	burst_mode = true;

	for (size_t c = 0; c < stripN; c++) {
		strip_type[c] = Strip::WS2812_RGB;
		lightguy::Strip::get(c).setStripType(Strip::Type(strip_type[c]));
	}

	int32_t counter = 0;
	for (size_t c = 0; c < stripN; c++) {
		for (size_t d = 0; d < universeN; d++) {
			universe[c][d] = counter++;
		}
	}
	for (size_t c = 0; c < stripN; c++) {
		striplen[c] = lightguy::Strip::get(c).getMaxPixelLen();
		lightguy::Strip::get(c).setPixelLen(striplen[c]);
	}

	readFlash();

	lightguy::Strip::get(0).dmaTransferFunc = [](const uint8_t *data, size_t len) {
      (void)data;
      (void)len;
	};
	lightguy::Strip::get(0).dmaBusyFunc = []() {
        return false;
	};

	lightguy::Strip::get(1).dmaTransferFunc = [](const uint8_t *data, size_t len) {
      (void)data;
      (void)len;
	};
	lightguy::Strip::get(0).dmaBusyFunc = []() {
        return false;
	};

	Driver::instance().setRGBW16(0x0000, 0x0000, 0x0000, 0x0000);
}

void Model::setOutputConfig(OutputConfig outputConfig) {
	output_config = outputConfig;
}

Model &Model::instance() {
	static Model model;
	if (!model.initialized) {
		model.initialized = true;
		model.init();
	}
	return model;
}

void Model::transferNow() {
	for (size_t c = 0; c < lightguy::Model::stripN; c++) {
		lightguy::Strip::get(c).transfer();
	}
}

void Model::setOutputData(uint8_t channel, const uint8_t *data, size_t len) {
	switch(output_config) {
	case OUTPUT_CONFIG_DUAL_STRIP: {
		Driver::instance().setRGBW16(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
		channel %= stripN;
		lightguy::Strip::get(channel).setData(data, len);
		lightguy::Strip::get(channel).transfer();
	} break;
	case OUTPUT_CONFIG_RGB_STRIP: {
		if (channel == 0) {
			if (len >= 3) {
				Driver::instance().setRGBW8CIE(data[0], data[1], data[2], 0xFF);
			}
		}
		if (channel == 1) {
			lightguy::Strip::get(channel).setData(data, len);
			lightguy::Strip::get(channel).transfer();
		}
	} break;
	case OUTPUT_CONFIG_RGBW: {
		if (channel == 0) {
			if (len >= 4) {
				Driver::instance().setRGBW8CIE(data[0], data[1], data[2], data[3]);
			}
		}
	} break;
	}
}

void Model::setUniverseOutputData(uint16_t uni, const uint8_t *data, size_t len) {
	switch(output_config) {
	case OUTPUT_CONFIG_DUAL_STRIP: {
		Driver::instance().setRGBW16(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
		for (size_t c = 0; c < stripN; c++) {
			bool set = false;
			for (size_t d = 0; d < universeN; d++) {
				if (universe[c][d] == uni) {
					lightguy::Strip::get(c).setUniverseData(d, data, len);
					set = true;
				}
			}
			if (set) {
				lightguy::Strip::get(c).transfer();
			}
		}
	} break;
	case OUTPUT_CONFIG_RGB_STRIP: {
		if (universe[0][0] == uni) {
			if (len >= 3) {
				Driver::instance().setRGBW8CIE(data[0], data[1], data[2], 0xFF);
			}
		}
		for (size_t c = 1; c < stripN; c++) {
			bool set = false;
			for (size_t d = 0; d < universeN; d++) {
				if (universe[c][d] == uni) {
					lightguy::Strip::get(c).setUniverseData(d, data, len);
					set = true;
				}
			}
			if (set) {
				lightguy::Strip::get(c).transfer();
			}
		}
	} break;
	case OUTPUT_CONFIG_RGBW: {
		if (universe[0][0] == uni) {
			if (len >= 4) {
				Driver::instance().setRGBW8CIE(data[0], data[1], data[2], data[3]);
			}
		}
	} break;
	}
}

} /* namespace lightguy */

