#include <stdint.h>
#include <string.h>

#include "./main.h"
#include "./control.h"
#include "./driver.h"
#include "./strip.h"
#include "./spi.h"

namespace lightkraken {

Control &Control::instance() {
    static Control control;
    if (!control.initialized) {
        control.initialized = true;
        control.init();
    }
    return control;
}

void Control::sync() {
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
		for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
			lightkraken::Strip::get(c).transfer();
		}
	} break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
		Driver::instance().sync(0);
		for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
			lightkraken::Strip::get(c).transfer();
		}
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
		Driver::instance().sync(0);
		for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
			lightkraken::Strip::get(c).transfer();
		}
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
		Driver::instance().sync(0);
		for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
			lightkraken::Strip::get(c).transfer();
		}
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
		for (size_t c = 0; c < Model::analogN; c++) {
			Driver::instance().sync(c);
		}
	} break;
	}
}

void Control::setUniverseOutputDataForDriver(size_t terminals, size_t components, uint16_t uni, const uint8_t *data, size_t len) {
    rgbww rgb[Driver::terminalN];
    for (size_t c = 0; c < terminals; c++) {
        rgb[c] = Driver::instance().rgbwwCIE(c);
    }
    for (size_t c = 0; c < terminals; c++) {
        switch(components) {
        case 5: {
            if (len > Model::instance().analogConfig(c).components[4].offset) {
                if(Model::instance().analogConfig(c).components[4].universe == uni) {
                    rgb[c].ww = data[Model::instance().analogConfig(c).components[4].offset];
                }
            }
        } 
        [[fallthrough]];
        case 4: {
            if (len > Model::instance().analogConfig(c).components[3].offset) {
                if(Model::instance().analogConfig(c).components[3].universe == uni) {
                    rgb[c].w = data[Model::instance().analogConfig(c).components[3].offset];
                }
            }
        } 
        [[fallthrough]];
        case 3: {
            if (len > Model::instance().analogConfig(c).components[2].offset) {
                if(Model::instance().analogConfig(c).components[2].universe == uni) {
                    rgb[c].b = data[Model::instance().analogConfig(c).components[2].offset];
                }
            }
        } 
        [[fallthrough]];
        case 2: {
            if (len > Model::instance().analogConfig(c).components[1].offset) {
                if(Model::instance().analogConfig(c).components[1].universe == uni) {
                    rgb[c].g = data[Model::instance().analogConfig(c).components[1].offset];
                }
            }
        } 
        [[fallthrough]];
        case 1: {
            if (len > Model::instance().analogConfig(c).components[0].offset) {
                if(Model::instance().analogConfig(c).components[0].universe == uni) {
                    rgb[c].r = data[Model::instance().analogConfig(c).components[0].offset];
                }
            }
        } 
        [[fallthrough]];
        default:
        case 0:
        break;
        }
    }
    for (size_t c = 0; c < terminals; c++) {
        Driver::instance().setRGBWWCIE(c,rgb[c]);
		if (!syncMode) {
			Driver::instance().sync(c);
		}
    }
}

void Control::setUniverseOutputData(uint16_t uni, const uint8_t *data, size_t len, bool nodriver) {
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
        for (size_t c = 0; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().universeStrip(c,d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len);
                    set = true;
                }
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
        if (!nodriver) {
        	setUniverseOutputDataForDriver(1, 3, uni, data, len);
        }
        for (size_t c = 0; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().universeStrip(c,d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len);
                    set = true;
                }
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
        if (!nodriver) {
        	setUniverseOutputDataForDriver(1, 3, uni, data, len);
        }
        for (size_t c = 1; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().universeStrip(c,d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len);
                    set = true;
                }
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
        if (!nodriver) {
        	setUniverseOutputDataForDriver(1, 4, uni, data, len);
        }
        for (size_t c = 1; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().universeStrip(c,d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len);
                    set = true;
                }
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
        if (!nodriver) {
        	setUniverseOutputDataForDriver(Model::analogN, 3, uni, data, len);
		}
    } break;
    }
}

void Control::init() {
    lightkraken::Strip::get(0).dmaTransferFunc = [](const uint8_t *data, size_t len) {
        SPI_0::instance().transfer(data, len, lightkraken::Strip::get(0).needsClock());
    };
    lightkraken::Strip::get(0).dmaBusyFunc = []() {
        return false;
    };
    lightkraken::Strip::get(1).dmaTransferFunc = [](const uint8_t *data, size_t len) {
        SPI_2::instance().transfer(data, len, lightkraken::Strip::get(1).needsClock());
    };
    lightkraken::Strip::get(1).dmaBusyFunc = []() {
        return false;
    };
    DEBUG_PRINTF(("Control up.\n"));
}

}  // namespace lightkraken {
