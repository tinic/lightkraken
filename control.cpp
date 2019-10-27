#include <stdint.h>
#include <string.h>

#include "./main.h"
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

void Control::setUniverseOutputDataForDriver(size_t terminals, size_t components, uint16_t uni, const uint8_t *data, size_t len) {
    rgbww rgb[Driver::terminalN];
    for (size_t c = 0; c < terminals; c++) {
        rgb[c] = Driver::instance().rgbwwCIE(c);
    }
    for (size_t c = 0; c < terminals; c++) {
        size_t offset = 0;
        switch(components) {
        case 5: {
            if(Model::instance().analogConfig(c).components[4].universe == uni) {
                rgb[c].ww = data[Model::instance().analogConfig(c).components[4].offset];
            }
        } 
        [[fallthrough]];
        case 4: {
            if(Model::instance().analogConfig(c).components[3].universe == uni) {
                rgb[c].w = data[Model::instance().analogConfig(c).components[3].offset];
            }
        } 
        [[fallthrough]];
        case 3: {
            if(Model::instance().analogConfig(c).components[2].universe == uni) {
                rgb[c].b = data[Model::instance().analogConfig(c).components[2].offset];
            }
        } 
        [[fallthrough]];
        case 2: {
            if(Model::instance().analogConfig(c).components[1].universe == uni) {
                rgb[c].g = data[Model::instance().analogConfig(c).components[1].offset];
            }
        } 
        [[fallthrough]];
        case 1: {
            if(Model::instance().analogConfig(c).components[0].universe == uni) {
                rgb[c].r = data[Model::instance().analogConfig(c).components[0].offset];
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
    }
}

void Control::setUniverseOutputData(uint16_t uni, const uint8_t *data, size_t len) {
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
        for (size_t c = 0; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().universeStrip(c,d) == uni) {
                    lightguy::Strip::get(c).setUniverseData(d, data, len);
                    set = true;
                }
            }
            if (set) {
                lightguy::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {

        setUniverseOutputDataForDriver(1, 3, uni, data, len);

        for (size_t c = 0; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().universeStrip(c,d) == uni) {
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

        setUniverseOutputDataForDriver(1, 3, uni, data, len);

        for (size_t c = 0; c < 1; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().universeStrip(c,d) == uni) {
                    lightguy::Strip::get(c).setUniverseData(d, data, len);
                    set = true;
                }
            }
            if (set) {
                lightguy::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {

        setUniverseOutputDataForDriver(1, 4, uni, data, len);
        
        for (size_t c = 0; c < 1; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().universeStrip(c,d) == uni) {
                    lightguy::Strip::get(c).setUniverseData(d, data, len);
                    set = true;
                }
            }
            if (set) {
                lightguy::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
        setUniverseOutputDataForDriver(2, 3, uni, data, len);
    } break;
    }
}

void Control::init() {
    lightguy::Strip::get(0).dmaTransferFunc = [](const uint8_t *data, size_t len) {
        SPI_2::instance().transfer(data, len, lightguy::Strip::get(0).needsClock());
    };
    lightguy::Strip::get(0).dmaBusyFunc = []() {
        return false;
    };

    lightguy::Strip::get(1).dmaTransferFunc = [](const uint8_t *data, size_t len) {
        SPI_0::instance().transfer(data, len, lightguy::Strip::get(1).needsClock());
    };
    lightguy::Strip::get(1).dmaBusyFunc = []() {
        return false;
    };

    DEBUG_PRINTF(("Control up.\n"));
}

}  // namespace lightguy {
