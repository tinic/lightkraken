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

enum PowerClass {
	PSE_TYPE_1_2_CLASS_0_3  = 0b111,
	PSE_TYPE_2_CLASS_4      = 0b101,
	PSE_TYPE_3_4_CLASS_0_3  = 0b110,
	PSE_TYPE_3_4_CLASS_4    = 0b100,
	PSE_TYPE_3_4_CLASS_5_6  = 0b010,
	PSE_TYPE_4_CLASS_7_8    = 0b000,
};

void Model::updateUserLED() {
	readPowerState();
	if (!powergood_state) {
		setUserLED(0x1f, 0x00, 0x00);
		return;
	}
	PowerClass pclass = (PowerClass)(((tph_state)?0x4:0x0)|
								 	 ((tpl_state)?0x2:0x0)|
									 (( bt_state)?0x0:0x1));
	switch(pclass) {
		default:
		setUserLED(0x00, 0x00, 0xff);
		break;
		case PSE_TYPE_3_4_CLASS_0_3:
		case PSE_TYPE_1_2_CLASS_0_3:
		setUserLED(0x1f, 0x0f, 0x00);
		break;
		case PSE_TYPE_3_4_CLASS_4:
		case PSE_TYPE_2_CLASS_4:
		setUserLED(0x1f, 0x1f, 0x00);
		break;
		case PSE_TYPE_3_4_CLASS_5_6:
		case PSE_TYPE_4_CLASS_7_8:
		setUserLED(0x00, 0x1f, 0x00);
		break;
	}
}

__attribute__ ((hot, optimize("O2")))
void Model::setUserLED(uint8_t r, uint8_t g, uint8_t b) {
	constexpr int32_t one = 20; // for 108Mhz
	constexpr int32_t t0l = (one * 35) / 130;
	constexpr int32_t t0h = (one * 95) / 130;
	constexpr int32_t t1l = (one * 70) / 130;
	constexpr int32_t t1h = (one * 60) / 130;
	uint32_t bits = (uint32_t(g)<<16) | (uint32_t(r)<<8) | uint32_t(b);
	for (int32_t d=23; d>=0; d--) {
		if ((1UL<<d) & bits) {
			// one
			for (int32_t c =0 ; c<t1l; c++) {
              GPIO_BOP(GPIOB) = GPIO_PIN_6;
			}
			for (int32_t c =0 ; c<t1h; c++) {
              GPIO_BC(GPIOB) = GPIO_PIN_6;
			}
		} else {
			// zero
			for (int32_t c =0 ; c<t0l; c++) {
              GPIO_BOP(GPIOB) = GPIO_PIN_6;
			}
			for (int32_t c =0 ; c<t0h; c++) {
              GPIO_BC(GPIOB) = GPIO_PIN_6;
			}
		}
	}
    GPIO_BC(GPIOB) = GPIO_PIN_6;
}

void Model::readPowerState() {
	bt_state = gpio_input_bit_get(GPIOA, GPIO_PIN_3) == RESET ? false : true;
	tpl_state = gpio_input_bit_get(GPIOA, GPIO_PIN_5) == RESET ? false : true;
	tph_state = gpio_input_bit_get(GPIOA, GPIO_PIN_4) == RESET ? false : true;
	powergood_state = gpio_input_bit_get(GPIOB, GPIO_PIN_0) == RESET ? false : true;
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

