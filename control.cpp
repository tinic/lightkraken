#include <stdint.h>
#include <memory.h>

#include "./control.h"
#include "./driver.h"
#include "./strip.h"
#include "./spi.h"

namespace lightguy {

Control &Control::instance() {
	static Control control;
	if (!control.initialized) {
		control.initialized = true;
		control.init();
	}
	return control;
}

void Control::transferNow() {
	for (size_t c = 0; c < lightguy::Model::stripN; c++) {
		lightguy::Strip::get(c).transfer();
	}
}

void Control::setOutputData(uint8_t channel, const uint8_t *data, size_t len) {
	switch(Model::instance().outputConfig()) {
	case Model::OUTPUT_CONFIG_DUAL_STRIP: {
		Driver::instance().setRGBW16(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
		channel %= Model::stripN;
		lightguy::Strip::get(channel).setData(data, len);
		lightguy::Strip::get(channel).transfer();
	} break;
	case Model::OUTPUT_CONFIG_RGB_STRIP: {
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
	case Model::OUTPUT_CONFIG_RGBW: {
		if (channel == 0) {
			if (len >= 4) {
				Driver::instance().setRGBW8CIE(data[0], data[1], data[2], data[3]);
			}
		}
	} break;
	}
}

void Control::setUniverseOutputData(uint16_t uni, const uint8_t *data, size_t len) {
	switch(Model::instance().outputConfig()) {
	case Model::OUTPUT_CONFIG_DUAL_STRIP: {
		Driver::instance().setRGBW16(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
		for (size_t c = 0; c < Model::stripN; c++) {
			bool set = false;
			for (size_t d = 0; d < Model::universeN; d++) {
				if (Model::instance().universe(c,d) == uni) {
					lightguy::Strip::get(c).setUniverseData(d, data, len);
					set = true;
				}
			}
			if (set) {
				lightguy::Strip::get(c).transfer();
			}
		}
	} break;
	case Model::OUTPUT_CONFIG_RGB_STRIP: {
		if (Model::instance().universe(0,0) == uni) {
			if (len >= 3) {
				Driver::instance().setRGBW8CIE(data[0], data[1], data[2], 0xFF);
			}
		}
		for (size_t c = 1; c < Model::stripN; c++) {
			bool set = false;
			for (size_t d = 0; d < Model::universeN; d++) {
				if (Model::instance().universe(c,d) == uni) {
					lightguy::Strip::get(c).setUniverseData(d, data, len);
					set = true;
				}
			}
			if (set) {
				lightguy::Strip::get(c).transfer();
			}
		}
	} break;
	case Model::OUTPUT_CONFIG_RGBW: {
		if (Model::instance().universe(0,0) == uni) {
			if (len >= 4) {
				Driver::instance().setRGBW8CIE(data[0], data[1], data[2], data[3]);
			}
		}
	} break;
	}
}

void Control::init() {
	for (size_t c = 0; c < Model::stripN; c++) {
		lightguy::Strip::get(c).setPixelLen(lightguy::Strip::get(c).getMaxPixelLen());
	}

	lightguy::Strip::get(0).dmaTransferFunc = [](const uint8_t *data, size_t len) {
		SPI_0::instance().dma_transfer(data, len);
	};
	lightguy::Strip::get(0).dmaBusyFunc = []() {
        return false;
	};

	lightguy::Strip::get(1).dmaTransferFunc = [](const uint8_t *data, size_t len) {
		SPI_2::instance().dma_transfer(data, len);
	};
	lightguy::Strip::get(1).dmaBusyFunc = []() {
        return false;
	};

    printf("Control up.");
}

}  // namespace lightguy {
