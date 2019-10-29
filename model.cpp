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
#include "./control.h"
#include "./spi.h"

namespace lightkraken {

constexpr static size_t settings_page_mem = 0x08000000 + 255 * 1024;

void Model::readFlash() {
	uint32_t *src = reinterpret_cast<uint32_t *>(settings_page_mem);
	uint32_t *dst = reinterpret_cast<uint32_t *>(this);
	if (*src != currentModelVersion) {
		return;
	}
	for (size_t c = 0; c < sizeof(Model); c += sizeof(uint32_t)) {
		*dst++ = *src++;
	}
}

void Model::writeFlash() {
	fmc_unlock();

	fmc_flag_clear(FMC_FLAG_BANK0_END);
	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

	fmc_page_erase(settings_page_mem);

	fmc_flag_clear(FMC_FLAG_BANK0_END);
	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

	uint32_t *src = reinterpret_cast<uint32_t *>(this);
	for (size_t c = 0; c < sizeof(Model); c += sizeof(uint32_t)) {
		fmc_word_program(settings_page_mem + c, *src++);

        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR); 
	}

	fmc_lock();
}

void Model::save() {
	writeFlash();
}

void Model::load() {
	readFlash();
}

void Model::reset() {
	defaults();
	save();
}

void Model::defaults() {
	model_version = currentModelVersion;

    const uint8_t IP_ADDRESS0 =  169;
    const uint8_t IP_ADDRESS1 =  254;
    const uint8_t IP_ADDRESS2 = 0x1e;
    const uint8_t IP_ADDRESS3 = 0xd5;

    const uint8_t IP_NETMASK0 =  255;
    const uint8_t IP_NETMASK1 =  255;
    const uint8_t IP_NETMASK2 =	   0;
    const uint8_t IP_NETMASK3 =	   0;

    const uint8_t IP_GATEWAY0 =  169;
    const uint8_t IP_GATEWAY1 =  254;
    const uint8_t IP_GATEWAY2 =	   0;
    const uint8_t IP_GATEWAY3 =	   1;

    IP4_ADDR(&ip4_address, IP_ADDRESS0, IP_ADDRESS1, IP_ADDRESS2, IP_ADDRESS3);
    IP4_ADDR(&ip4_netmask, IP_NETMASK0, IP_NETMASK1, IP_NETMASK2, IP_NETMASK3);
    IP4_ADDR(&ip4_gateway, IP_GATEWAY0, IP_GATEWAY1, IP_GATEWAY2, IP_GATEWAY3);

    dhcp = true;
    
    receive_broadcast = false;

    glob_pwmlimit = 1.0f;
    glob_illum = 0x1F;
    glob_comp_lim = 0xFF;

    output_config = OUTPUT_CONFIG_RGBW_STRIP;

    burst_mode = false;

    int32_t counter = 0;
    for (size_t c = 0; c < stripN; c++) {
        strip_config[c].type = Strip::GS8208_RGB;
        lightkraken::Strip::get(c).setStripType(Strip::Type(strip_config[c].type));
        strip_config[c].len = 256;
        strip_config[c].color = rgb8();
        lightkraken::Strip::get(c).setPixelLen(strip_config[c].len);
        for (size_t d = 0; d < universeN; d++) {
            strip_config[c].universe[d] = counter++;
        }
    }

    counter = 0;
    for (size_t c = 0; c < analogN; c++) {
        analog_config[c].type = 0;
        for (size_t d = 0; d < analogCompN; d++) {
            analog_config[c].components[d].value = 0;
            analog_config[c].components[d].universe = 0;
            analog_config[c].components[d].offset = counter++;
        }
    }
}

void Model::applyColor() {
    for (size_t c = 0; c < analogN; c++) {
        rgbww col;
        col.r = analog_config[c].components[0].value;
        col.g = analog_config[c].components[1].value;
        col.b = analog_config[c].components[2].value;
        col.w = analog_config[c].components[3].value;
        col.ww = analog_config[c].components[4].value;
        Driver::instance().setRGBWWCIE(c, col);
    }

    uint8_t buf[512];
    for (size_t c = 0; c < stripN; c++) {
        size_t cpp = lightkraken::Strip::get(c).getComponentsPerPixel();
        size_t len = 0;
        switch(cpp) {
            case 3: {
                for (int32_t d = 0; d < 510; d += 3) {
                    buf[d + 0] = (strip_config[c].color.b) & 0xFF;
                    buf[d + 1] = (strip_config[c].color.g) & 0xFF;
                    buf[d + 2] = (strip_config[c].color.r) & 0xFF;
                    len += 3;
                }
            } break;
            case 4: {
                for (int32_t d = 0; d < 512; d += 4) {
                    buf[d + 0] = (strip_config[c].color.b) & 0xFF;
                    buf[d + 1] = (strip_config[c].color.g) & 0xFF;
                    buf[d + 2] = (strip_config[c].color.r) & 0xFF;
                    buf[d + 3] = (strip_config[c].color.x) & 0xFF;
                    len += 4;
                }
            } break;
        }
        for (size_t d = 0; d < universeN; d++) {
            Control::instance().setUniverseOutputData(strip_config[c].universe[d], buf, len, true);
        }
    }
}

void Model::init() {
	defaults();
    readFlash();
    applyColor();
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

} /* namespace lightkraken */

