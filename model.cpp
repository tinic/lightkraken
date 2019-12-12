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
extern "C" {
#include "gd32f10x.h"
}

#include "./model.h"
#include "./strip.h"
#include "./driver.h"
#include "./control.h"
#include "./spi.h"
#include "./systick.h"

extern "C" {

__attribute__((used))
void EXTI1_IRQHandler(void) {
	static uint32_t start_time = ~uint32_t(0);
	uint32_t now = lightkraken::Systick::instance().systemTime();
	if (gpio_input_bit_get(GPIOB, GPIO_PIN_1) == RESET) {
		start_time = now;
	} else {
		if (start_time !=  ~uint32_t(0)) {
			if ((now - start_time) > 10000 &&
			    (now - start_time) < 20000) {
			    lightkraken::Model::instance().reset();
				NVIC_SystemReset();
			}
			start_time = ~uint32_t(0);
		}
	}
	exti_interrupt_flag_clear(EXTI_1);
}

}

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
    const uint8_t IP_NETMASK2 =    0;
    const uint8_t IP_NETMASK3 =    0;

    const uint8_t IP_GATEWAY0 =  169;
    const uint8_t IP_GATEWAY1 =  254;
    const uint8_t IP_GATEWAY2 =    0;
    const uint8_t IP_GATEWAY3 =    1;

    IP4_ADDR(&ip4_address, IP_ADDRESS0, IP_ADDRESS1, IP_ADDRESS2, IP_ADDRESS3);
    IP4_ADDR(&ip4_netmask, IP_NETMASK0, IP_NETMASK1, IP_NETMASK2, IP_NETMASK3);
    IP4_ADDR(&ip4_gateway, IP_GATEWAY0, IP_GATEWAY1, IP_GATEWAY2, IP_GATEWAY3);

    dhcp = true;
    
    receive_broadcast = false;

    output_config = OUTPUT_CONFIG_DUAL_STRIP;
    output_mode = MODE_MAIN_LOOP;

    burst_mode = true;

    int32_t artnetcounter = 0;
    int32_t e131counter = 1;
    memset(strip_config, 0, sizeof(strip_config));
    for (size_t c = 0; c < stripN; c++) {
        strip_config[c].output_type = Strip::GS8208_RGB;
        strip_config[c].input_type = Strip::INPUT_dRGB8;
        strip_config[c].comp_limit = 1.0f;
        strip_config[c].glob_illum = 1.0f;
        lightkraken::Strip::get(c).setStripType(Strip::OutputType(strip_config[c].output_type));
        strip_config[c].len = 256;
        strip_config[c].color = rgb8();
        strip_config[c].rgbSpace.setsRGB();
        lightkraken::Strip::get(c).setPixelLen(strip_config[c].len);
        for (size_t d = 0; d < universeN; d++) {
            strip_config[c].artnet[d] = artnetcounter++;
            strip_config[c].e131[d] = e131counter++;
        }
    }

    int32_t counter = 1;
    memset(analog_config, 0, sizeof(analog_config));
    for (size_t c = 0; c < analogN; c++) {
        analog_config[c].rgbSpace.setsRGB();
        analog_config[c].pwm_limit = 1.0f;
        for (size_t d = 0; d < analogCompN; d++) {
            analog_config[c].components[d].artnet.channel = counter;
            analog_config[c].components[d].artnet.universe = 0;
            analog_config[c].components[d].e131.channel = counter++;
            analog_config[c].components[d].e131.universe = 1;
            analog_config[c].components[d].value = 0;
        }
    }
}

void Model::apply() {


    for (size_t c = 0; c < stripN; c++) {
    strip_config[c].rgbSpace.setsRGB();
        lightkraken::Strip::get(c).setStripType(Strip::OutputType(strip_config[c].output_type));
        lightkraken::Strip::get(c).setPixelLen(strip_config[c].len);
        lightkraken::Strip::get(c).setRGBColorSpace(strip_config[c].rgbSpace);
        lightkraken::Strip::get(c).setCompLimit(strip_config[c].comp_limit);
        lightkraken::Strip::get(c).setGlobIllum(strip_config[c].glob_illum);
    }

    for (size_t c = 0; c < analogN; c++) {
        rgbww col;
        col.r = analog_config[c].components[0].value;
        col.g = analog_config[c].components[1].value;
        col.b = analog_config[c].components[2].value;
        col.w = analog_config[c].components[3].value;
        col.ww = analog_config[c].components[4].value;
        Driver::instance().setsRGBWWCIE(c, col);
        Driver::instance().setRGBColorSpace(c, analog_config[c].rgbSpace);
        Driver::instance().setPWMLimit(c, analog_config[c].pwm_limit);
    }

    Control::instance().setColor();

    if (output_mode == MODE_INTERRUPT) {
        lightkraken::Control::instance().syncFromInterrupt(lightkraken::SPI_0::instance());
        lightkraken::Control::instance().syncFromInterrupt(lightkraken::SPI_2::instance());
    } else {
        lightkraken::Control::instance().sync();
    }
}

void Model::setTag(const char *str) { 
    strncpy(tag_str, str, sizeof(tag_str) - 1); 
    tag_str[sizeof(tag_str)-1] = 0;
}

void Model::init() {
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_1);

	nvic_irq_enable(EXTI1_IRQn, 2U, 1U);
	gpio_exti_source_select(GPIO_EVENT_PORT_GPIOB, GPIO_EVENT_PIN_1);
	exti_init(EXTI_1, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_interrupt_flag_clear(EXTI_1);

    defaults();
    readFlash();
    
    Systick::instance().scheduleApply();
}

void Model::setOutputConfig(OutputConfig outputConfig) {
    output_config = std::clamp(outputConfig, OUTPUT_CONFIG_DUAL_STRIP, OUTPUT_CONFIG_RGBWWW);
}

void Model::setOutputMode(OutputMode outputMode) {
    output_mode = std::clamp(outputMode, MODE_MAIN_LOOP, MODE_INTERRUPT);
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

