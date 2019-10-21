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

void Control::setUniverseOutputDataForDriver(size_t channels, uint16_t uni, const uint8_t *data, size_t len) {
    rgb8 rgb[Driver::terminalN];
    for (size_t c = 0; c < Driver::terminalN; c++) {
        rgb[c] = Driver::instance().rgb8CIE(c);
    }
    for (size_t c = 0; c < channels; c++) {
        size_t offset = 0;
        if (Model::instance().universeOffset(c,offset) == uni) {
            if (len >= offset) {
                switch(c%3) {
                case 0: {
                    rgb[c/3].r = data[offset];
                    } break;
                case 1: {
                    rgb[c/3].g = data[offset];
                    } break;
                case 2: {
                    rgb[c/3].b = data[offset];
                    } break;
                }
            }
        }
    }
    for (size_t c = 0; c < Driver::terminalN; c++) {
        Driver::instance().setRGB8CIE(c,rgb[c]);
        
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
    case Model::OUTPUT_CONFIG_RGB_STRIP: {

        setUniverseOutputDataForDriver(3, uni, data, len);

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

        setUniverseOutputDataForDriver(4, uni, data, len);
        
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
    case Model::OUTPUT_CONFIG_RGBW: {
        setUniverseOutputDataForDriver(4, uni, data, len);
    } break;
    case Model::OUTPUT_CONFIG_RGBWW: {
        setUniverseOutputDataForDriver(5, uni, data, len);
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
        setUniverseOutputDataForDriver(6, uni, data, len);
    } break;
    }
}

void Control::init() {
    for (size_t c = 0; c < Model::stripN; c++) {
        lightguy::Strip::get(c).setPixelLen(lightguy::Strip::get(c).getMaxPixelLen());
    }

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

    printf("Control up.\n");
}

}  // namespace lightguy {
