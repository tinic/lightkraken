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
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "./main.h"
#include "./control.h"
#include "./driver.h"
#include "./strip.h"
#include "./spi.h"
#include "./perf.h"
#include "./systick.h"

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
    case Model::OUTPUT_CONFIG_RGBWWW: {
        for (size_t c = 0; c < 1; c++) {
            Driver::instance().sync(c);
        }
    } break;
    default: {
    } break;
    }
}

void Control::collectAllActiveArtnetUniverses(std::array<uint16_t, Model::maxUniverses> &universes, size_t &universeCount) {
    universeCount = 0;
    class UniqueCollector {
    public:
        UniqueCollector() {
            memset(&collected_universes[0], 0xFF, sizeof(collected_universes));
        }
    
        void maybeAcquire(uint16_t universe) {
            for (size_t c = 0; c < Model::maxUniverses; c++) {
                if (collected_universes[c] == universe) {
                    return;
                }
                if (collected_universes[c] == 0xFFFF) {
                    collected_universes[c] = universe;
                    return;
                }
            }
        }

        void fillArray(std::array<uint16_t, Model::maxUniverses> &universes, size_t &universeCount) {
            for (size_t c = 0; c < Model::maxUniverses; c++) {
                if (collected_universes[c] == 0xFFFF) {
                    return;
                }
                universes[universeCount++] = collected_universes[c];
            }
        }
    private:
        uint16_t collected_universes[Model::maxUniverses];
    } uniqueCollector;

    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
        for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Strip::get(c).isUniverseActive(d, Strip::InputType(Model::instance().stripConfig(c).input_type))) {
                    uniqueCollector.maybeAcquire(Model::instance().artnetStrip(c,d));
                }
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
        for (size_t c = 0; c < 3; c++) { 
            uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[c].artnet.universe);
        }
        for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Strip::get(c).isUniverseActive(d, Strip::InputType(Model::instance().stripConfig(c).input_type))) {
                    uniqueCollector.maybeAcquire(Model::instance().artnetStrip(c, d));
                }
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
        for (size_t c = 0; c < 3; c++) { 
            uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[c].artnet.universe);
        }
        for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Strip::get(c).isUniverseActive(d, Strip::InputType(Model::instance().stripConfig(c).input_type))) {
                    uniqueCollector.maybeAcquire(Model::instance().artnetStrip(c, d));
                }
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
        for (size_t c = 0; c < 4; c++) { 
            uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[c].artnet.universe);
        }
        for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Strip::get(c).isUniverseActive(d, Strip::InputType(Model::instance().stripConfig(c).input_type))) {
                    uniqueCollector.maybeAcquire(Model::instance().artnetStrip(c, d));
                }
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
        for (size_t c = 0; c < Model::analogN; c++) {
            for (size_t d = 0; d < 3; d++) { 
                uniqueCollector.maybeAcquire(Model::instance().analogConfig(c).components[d].artnet.universe);
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBWWW: {
        for (size_t c = 0; c < 1; c++) {
            for (size_t d = 0; d < 5; d++) { 
                uniqueCollector.maybeAcquire(Model::instance().analogConfig(c).components[d].artnet.universe);
            }
        }
    } break;
    default: {
    } break;
    }
    uniqueCollector.fillArray(universes, universeCount);
}

void Control::collectAllActiveE131Universes(std::array<uint16_t, Model::maxUniverses> &universes, size_t &universeCount) {
    universeCount = 0;
    class UniqueCollector {
    public:
        UniqueCollector() {
            memset(&collected_universes[0], 0xFF, sizeof(collected_universes));
        }
    
        void maybeAcquire(uint16_t universe) {
            for (size_t c = 0; c < Model::maxUniverses; c++) {
                if (collected_universes[c] == universe) {
                    return;
                }
                if (collected_universes[c] == 0xFFFF) {
                    collected_universes[c] = universe;
                    return;
                }
            }
        }

        void fillArray(std::array<uint16_t, Model::maxUniverses> &universes, size_t &universeCount) {
            for (size_t c = 0; c < Model::maxUniverses; c++) {
                if (collected_universes[c] == 0xFFFF) {
                    return;
                }
                universes[universeCount++] = collected_universes[c];
            }
        }
    private:
        uint16_t collected_universes[Model::maxUniverses];
    } uniqueCollector;

    switch (Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
            for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
                for (size_t d = 0; d < Model::universeN; d++) {
                    if (Strip::get(c).isUniverseActive(d, Strip::InputType(Model::instance().stripConfig(c).input_type))) {
                        uniqueCollector.maybeAcquire(Model::instance().e131Strip(c, d));
                    }
                }
            }
        } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
            for (size_t c = 0; c < 3; c++) { 
                uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[c].e131.universe);
            }
            for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
                for (size_t d = 0; d < Model::universeN; d++) {
                    if (Strip::get(c).isUniverseActive(d, Strip::InputType(Model::instance().stripConfig(c).input_type))) {
                        uniqueCollector.maybeAcquire(Model::instance().e131Strip(c, d));
                    }
                }
            }
        } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
            for (size_t c = 0; c < 3; c++) { 
                uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[c].e131.universe);
            }
            for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
                for (size_t d = 0; d < Model::universeN; d++) {
                    if (Strip::get(c).isUniverseActive(d, Strip::InputType(Model::instance().stripConfig(c).input_type))) {
                        uniqueCollector.maybeAcquire(Model::instance().e131Strip(c, d));
                    }
                }
            }
        } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
            for (size_t c = 0; c < 4; c++) { 
                uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[c].e131.universe);
            }
            for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
                for (size_t d = 0; d < Model::universeN; d++) {
                    if (Strip::get(c).isUniverseActive(d, Strip::InputType(Model::instance().stripConfig(c).input_type))) {
                        uniqueCollector.maybeAcquire(Model::instance().e131Strip(c, d));
                    }
                }
            }
        } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
            for (size_t c = 0; c < Model::analogN; c++) {
                for (size_t d = 0; d < 3; d++) { 
                    uniqueCollector.maybeAcquire(Model::instance().analogConfig(c).components[d].e131.universe);
                }
            }
        } break;
    case Model::OUTPUT_CONFIG_RGBWWW: {
            for (size_t c = 0; c < 1; c++) {
                for (size_t d = 0; d < 5; d++) { 
                    uniqueCollector.maybeAcquire(Model::instance().analogConfig(c).components[d].e131.universe);
                }
            }
        } break;
    default: {
        } break;
    }
    uniqueCollector.fillArray(universes, universeCount);
}

void Control::interateAllActiveArtnetUniverses(std::function<void (uint16_t universe)> callback) {
    size_t universeCount = 0;
    std::array<uint16_t, Model::maxUniverses> universes;
    collectAllActiveArtnetUniverses(universes, universeCount);

    for (size_t c = 0; c < universeCount; c++) {
        callback(universes[c]);
    }
}

void Control::setArtnetUniverseOutputDataForDriver(size_t terminals, size_t components, uint16_t uni, const uint8_t *data, size_t len) {
    clearStartup();

    rgbww rgb[Driver::terminalN];
    for (size_t c = 0; c < terminals; c++) {
        rgb[c] = Driver::instance().srgbwwCIE(c);
    }
    for (size_t c = 0; c < terminals; c++) {
        switch(components) {
        case 5: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[4].artnet.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[4].artnet.universe == uni) {
                    rgb[c].ww = data[channel];
                }
            }
        } 
        [[fallthrough]];
        case 4: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[3].artnet.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[3].artnet.universe == uni) {
                    rgb[c].w = data[channel];
                }
            }
        } 
        [[fallthrough]];
        case 3: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[2].artnet.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[2].artnet.universe == uni) {
                    rgb[c].b = data[channel];
                }
            }
        } 
        [[fallthrough]];
        case 2: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[1].artnet.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[1].artnet.universe == uni) {
                    rgb[c].g = data[channel];
                }
            }
        } 
        [[fallthrough]];
        case 1: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[0].artnet.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[0].artnet.universe == uni) {
                    rgb[c].r = data[channel];
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
        Driver::instance().setRGBWW(c,rgb[c]);
        if (!syncMode) {
            Driver::instance().sync(c);
        }
    }
}

void Control::setE131UniverseOutputDataForDriver(size_t terminals, size_t components, uint16_t uni, const uint8_t *data, size_t len) {
    clearStartup();

    rgbww rgb[Driver::terminalN];
    for (size_t c = 0; c < terminals; c++) {
        rgb[c] = Driver::instance().srgbwwCIE(c);
    }
    for (size_t c = 0; c < terminals; c++) {
        switch(components) {
        case 5: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[4].e131.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[4].e131.universe == uni) {
                    rgb[c].ww = data[channel];
                }
            }
        } 
        [[fallthrough]];
        case 4: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[3].e131.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[3].e131.universe == uni) {
                    rgb[c].w = data[channel];
                }
            }
        } 
        [[fallthrough]];
        case 3: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[2].e131.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[2].e131.universe == uni) {
                    rgb[c].b = data[channel];
                }
            }
        } 
        [[fallthrough]];
        case 2: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[1].e131.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[1].e131.universe == uni) {
                    rgb[c].g = data[channel];
                }
            }
        } 
        [[fallthrough]];
        case 1: {
            size_t channel = size_t(std::clamp(Model::instance().analogConfig(c).components[0].e131.channel - 1, 0, 511));
            if (len > channel) {
                if (Model::instance().analogConfig(c).components[0].e131.universe == uni) {
                    rgb[c].r = data[channel];
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
        Driver::instance().setRGBWW(c,rgb[c]);
        if (!syncMode) {
            Driver::instance().sync(c);
        }
    }
}


void Control::setArtnetUniverseOutputData(uint16_t uni, const uint8_t *data, size_t len, bool nodriver) {
    clearStartup();

    PerfMeasure perf(PerfMeasure::SLOT_SET_DATA);
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
        for (size_t c = 0; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().artnetStrip(c,d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len, Strip::InputType(Model::instance().stripConfig(c).input_type));
                    set = true;
                }
            }
            if (set) {
                setDataReceived();
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
        if (!nodriver) {
            setArtnetUniverseOutputDataForDriver(1, 3, uni, data, len);
        }
        for (size_t c = 0; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().artnetStrip(c, d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len, Strip::InputType(Model::instance().stripConfig(c).input_type));
                    set = true;
                }
            }
            if (set) {
                setDataReceived();
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
        if (!nodriver) {
            setArtnetUniverseOutputDataForDriver(1, 3, uni, data, len);
        }
        for (size_t c = 1; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().artnetStrip(c, d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len, Strip::InputType(Model::instance().stripConfig(c).input_type));
                    set = true;
                }
            }
            if (set) {
                setDataReceived();
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
        if (!nodriver) {
            setArtnetUniverseOutputDataForDriver(1, 4, uni, data, len);
        }
        for (size_t c = 1; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().artnetStrip(c, d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len, Strip::InputType(Model::instance().stripConfig(c).input_type));
                    set = true;
                }
            }
            if (set) {
                setDataReceived();
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
        if (!nodriver) {
            setArtnetUniverseOutputDataForDriver(Model::analogN, 3, uni, data, len);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBWWW: {
        if (!nodriver) {
            setArtnetUniverseOutputDataForDriver(Model::analogN, 5, uni, data, len);
        }
    } break;
    }
}

void Control::setE131UniverseOutputData(uint16_t uni, const uint8_t *data, size_t len, bool nodriver) {
    clearStartup();

    PerfMeasure perf(PerfMeasure::SLOT_SET_DATA);
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
        for (size_t c = 0; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().e131Strip(c,d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len, Strip::InputType(Model::instance().stripConfig(c).input_type));
                    set = true;
                }
            }
            if (set) {
                setDataReceived();
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
        if (!nodriver) {
            setE131UniverseOutputDataForDriver(1, 3, uni, data, len);
        }
        for (size_t c = 0; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().e131Strip(c, d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len, Strip::InputType(Model::instance().stripConfig(c).input_type));
                    set = true;
                }
            }
            if (set) {
                setDataReceived();
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
        if (!nodriver) {
            setE131UniverseOutputDataForDriver(1, 3, uni, data, len);
        }
        for (size_t c = 1; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().e131Strip(c, d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len, Strip::InputType(Model::instance().stripConfig(c).input_type));
                    set = true;
                }
            }
            if (set) {
                setDataReceived();
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
        if (!nodriver) {
            setE131UniverseOutputDataForDriver(1, 4, uni, data, len);
        }
        for (size_t c = 1; c < Model::stripN; c++) {
            bool set = false;
            for (size_t d = 0; d < Model::universeN; d++) {
                if (Model::instance().e131Strip(c, d) == uni) {
                    lightkraken::Strip::get(c).setUniverseData(d, data, len, Strip::InputType(Model::instance().stripConfig(c).input_type));
                    set = true;
                }
            }
            if (set) {
                setDataReceived();
            }
            if (set && !syncMode) {
                lightkraken::Strip::get(c).transfer();
            }
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
        if (!nodriver) {
            setE131UniverseOutputDataForDriver(Model::analogN, 3, uni, data, len);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBWWW: {
        if (!nodriver) {
            setE131UniverseOutputDataForDriver(Model::analogN, 5, uni, data, len);
        }
    } break;
    }
}

void Control::setColor() {
    uint8_t buf[Strip::bytesMaxLen];
    for (size_t c = 0; c < Model::stripN; c++) {
        size_t cpp = lightkraken::Strip::get(c).getBytesPerPixel();
        size_t len = 0;
        switch(cpp) {
            case 3: {
                for (size_t d = 0; d <= sizeof(buf)-3; d += 3) {
                    buf[d + 0] = (Model::instance().stripConfig(c).color.r) & 0xFF;
                    buf[d + 1] = (Model::instance().stripConfig(c).color.g) & 0xFF;
                    buf[d + 2] = (Model::instance().stripConfig(c).color.b) & 0xFF;
                    len += 3;
                }
				lightkraken::Strip::get(c).setData(buf, len, Strip::INPUT_dRGB8);
            } break;
            case 4: {
                for (size_t d = 0; d <= sizeof(buf)-4; d += 4) {
                    buf[d + 0] = (Model::instance().stripConfig(c).color.r) & 0xFF;
                    buf[d + 1] = (Model::instance().stripConfig(c).color.g) & 0xFF;
                    buf[d + 2] = (Model::instance().stripConfig(c).color.b) & 0xFF;
                    buf[d + 3] = (Model::instance().stripConfig(c).color.x) & 0xFF;
                    len += 4;
                }
				lightkraken::Strip::get(c).setData(buf, len, Strip::INPUT_dRGBW8);
            } break;
            case 6: {
                for (size_t d = 0; d <= sizeof(buf)-6; d += 6) {
                    buf[d + 0] = 
                    buf[d + 1] = (Model::instance().stripConfig(c).color.r) & 0xFF;
                    buf[d + 2] = 
                    buf[d + 3] = (Model::instance().stripConfig(c).color.g) & 0xFF;
                    buf[d + 4] = 
                    buf[d + 5] = (Model::instance().stripConfig(c).color.b) & 0xFF;
                    len += 6;
                }
				lightkraken::Strip::get(c).setData(buf, len, Strip::INPUT_dRGB16MSB);
            } break;
        }
    }
}

void Control::update() {
	if (inStartup()) {
		startupModePattern();
		sync();
	} else if (color_scheduled) {
        color_scheduled = false;
        setColor();
        switch(Model::instance().outputConfig()) {
        case Model::OUTPUT_CONFIG_DUAL_STRIP: {
            for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
                lightkraken::Strip::get(c).transfer();
            }
        } break;
        case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
            for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
                lightkraken::Strip::get(c).transfer();
            }
        } break;
        case Model::OUTPUT_CONFIG_RGB_STRIP: {
            for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
                lightkraken::Strip::get(c).transfer();
            }
        } break;
        case Model::OUTPUT_CONFIG_RGBW_STRIP: {
            for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
                lightkraken::Strip::get(c).transfer();
            }
        } break;
        case Model::OUTPUT_CONFIG_RGB_RGB: {
        } break;
        case Model::OUTPUT_CONFIG_RGBWWW: {
        } break;
        default: {
        } break;
        }
    }

    lightkraken::SPI_0::instance().setFast(lightkraken::Strip::get(0).needsClock() == false);
    lightkraken::SPI_2::instance().setFast(lightkraken::Strip::get(1).needsClock() == false);
    
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
        lightkraken::SPI_2::instance().update();
        lightkraken::SPI_0::instance().update();
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
        lightkraken::SPI_2::instance().update();
        lightkraken::SPI_0::instance().update();
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
        lightkraken::SPI_2::instance().update();
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
        lightkraken::SPI_2::instance().update();
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
    } break;
    case Model::OUTPUT_CONFIG_RGBWWW: {
    } break;
    default: {
    } break;
    }
}

void Control::init() {

    lightkraken::Strip::get(0).dmaTransferFunc = [](const uint8_t *data, size_t len) {
        SPI_0::instance().transfer(data, len, lightkraken::Strip::get(0).needsClock());
    };
    lightkraken::Strip::get(0).dmaBusyFunc = []() {
        return SPI_0::instance().busy();
    };

    lightkraken::Strip::get(1).dmaTransferFunc = [](const uint8_t *data, size_t len) {
        SPI_2::instance().transfer(data, len, lightkraken::Strip::get(1).needsClock());
    };
    lightkraken::Strip::get(1).dmaBusyFunc = []() {
        return SPI_2::instance().busy();
    };
    
    DEBUG_PRINTF(("Control up.\n"));
}

void Control::startupModePattern() {
    PerfMeasure perf(PerfMeasure::SLOT_SET_DATA);
	auto effect = [=] (size_t strip) {
		switch (Model::instance().stripConfig(strip).startup_mode) {
			case Strip::STARTUP_MODE_COLOR: {	
				uint8_t buf[Strip::bytesMaxLen];
				size_t l = lightkraken::Strip::get(strip).getPixelLen();
                size_t cpp = lightkraken::Strip::get(strip).getBytesPerPixel();
				for (size_t c = 0; c < l; c++) {
                    switch(cpp) {
                        case 3: {
                            buf[c*3+0] = Model::instance().stripConfig(strip).color.r;
                            buf[c*3+1] = Model::instance().stripConfig(strip).color.g;
                            buf[c*3+2] = Model::instance().stripConfig(strip).color.b;
                        } break;
                        case 4: {
                            buf[c*4+0] = Model::instance().stripConfig(strip).color.r;
                            buf[c*4+1] = Model::instance().stripConfig(strip).color.g;
                            buf[c*4+2] = Model::instance().stripConfig(strip).color.b;
                            buf[c*4+3] = Model::instance().stripConfig(strip).color.x;
                        } break;
                        case 6: {
                            buf[c*6 + 0] = 
                            buf[c*6 + 1] = (Model::instance().stripConfig(strip).color.r) & 0xFF;
                            buf[c*6 + 2] = 
                            buf[c*6 + 3] = (Model::instance().stripConfig(strip).color.g) & 0xFF;
                            buf[c*6 + 4] = 
                            buf[c*6 + 5] = (Model::instance().stripConfig(strip).color.b) & 0xFF;
                        } break;
                    }
                }
                switch(cpp) {
                    default:
                    case 3: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGB8); break;
                    case 4: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGBW8); break;
                    case 6: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGB16MSB); break;
                }
			} break;
			case Strip::STARTUP_MODE_RAINBOW: {	
				uint8_t buf[Strip::bytesMaxLen];
				float h = 1.0f - fmod( Systick::instance().systemTime() / 10000.f, 1.0f);
				size_t l = lightkraken::Strip::get(strip).getPixelLen();
                size_t cpp = lightkraken::Strip::get(strip).getBytesPerPixel();
				for (size_t c = 0; c < l; c++) {
					hsv col_hsv(fmod(h + c * (1.0f / 255.0f), 1.0f), 1.0f, 1.0f);
					rgb col_rgb(col_hsv);
					rgb8 col_rgb8(col_rgb);
                    switch(cpp) {
                        case 3: {
                            buf[c*3+0] = col_rgb8.red();
                            buf[c*3+1] = col_rgb8.green();
                            buf[c*3+2] = col_rgb8.blue();
                        } break;
                        case 4: {
                            buf[c*4+0] = col_rgb8.red();
                            buf[c*4+1] = col_rgb8.green();
                            buf[c*4+2] = col_rgb8.blue();
                            buf[c*4+3] = 0;
                        } break;
                        case 6: {
                            buf[c*6 + 0] = 
                            buf[c*6 + 1] = col_rgb8.red();
                            buf[c*6 + 2] = 
                            buf[c*6 + 3] = col_rgb8.green();
                            buf[c*6 + 4] = 
                            buf[c*6 + 5] = col_rgb8.blue();
                        } break;
                    }
				}
                switch(cpp) {
                    default:
                    case 3: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGB8); break;
                    case 4: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGBW8); break;
                    case 6: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGB16MSB); break;
                }
			} break;
			case Strip::STARTUP_MODE_TRACER: {	
				uint8_t buf[Strip::bytesMaxLen];
				float h = 1.0f - fmod( Systick::instance().systemTime() / 5000.f, 1.0f);
				size_t l = lightkraken::Strip::get(strip).getPixelLen();
                size_t cpp = lightkraken::Strip::get(strip).getBytesPerPixel();
				for (size_t c = 0; c < l; c++) {
					size_t i = std::clamp(size_t(l * fmod(h + (c / float(l)), 1.0f)), size_t(0), l);
					if (i == 0) {
                        switch(cpp) {
                            case 3: {
                                buf[c*3+0] = 0xFF;
                                buf[c*3+1] = 0xFF;
                                buf[c*3+2] = 0xFF;
                            } break;
                            case 4: {
                                buf[c*4+0] = 0xFF;
                                buf[c*4+1] = 0xFF;
                                buf[c*4+2] = 0xFF;
                                buf[c*4+3] = 0xFF;
                            } break;
                            case 6: {
                                buf[c*6+0] = 0xFF;
                                buf[c*6+1] = 0xFF;
                                buf[c*6+2] = 0xFF;
                                buf[c*6+4] = 0xFF;
                                buf[c*6+5] = 0xFF;
                                buf[c*6+6] = 0xFF;
                            } break;
                        }
					} else {
                        switch(cpp) {
                            case 3: {
                                buf[c*3+0] = 0x00;
                                buf[c*3+1] = 0x00;
                                buf[c*3+2] = 0x00;
                            } break;
                            case 4: {
                                buf[c*4+0] = 0x00;
                                buf[c*4+1] = 0x00;
                                buf[c*4+2] = 0x00;
                                buf[c*4+3] = 0x00;
                            } break;
                            case 6: {
                                buf[c*6+0] = 0x00;
                                buf[c*6+1] = 0x00;
                                buf[c*6+2] = 0x00;
                                buf[c*6+3] = 0x00;
                                buf[c*6+4] = 0x00;
                                buf[c*6+5] = 0x00;
                            } break;
                        }
					}
				}
                switch(cpp) {
                    default:
                    case 3: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGB8); break;
                    case 4: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGBW8); break;
                    case 6: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGB16MSB); break;
                }
			} break;
			case Strip::STARTUP_MODE_SOLID_TRACER: {	
				uint8_t buf[Strip::bytesMaxLen];
				float h = 1.0f - fmod( Systick::instance().systemTime() / 5000.f, 1.0f);
				size_t l = lightkraken::Strip::get(strip).getPixelLen() + 1;
                size_t cpp = lightkraken::Strip::get(strip).getBytesPerPixel();
				for (size_t c = 0; c < l; c++) {
					size_t i = std::clamp(size_t(l * fmod(h + (c / float(l)), 1.0f)), size_t(0), l);
					if (c < i) {
                        switch(cpp) {
                            case 3: {
                                buf[c*3+0] = 0xFF;
                                buf[c*3+1] = 0xFF;
                                buf[c*3+2] = 0xFF;
                            } break;
                            case 4: {
                                buf[c*4+0] = 0xFF;
                                buf[c*4+1] = 0xFF;
                                buf[c*4+2] = 0xFF;
                                buf[c*4+3] = 0xFF;
                            } break;
                            case 6: {
                                buf[c*6+0] = 0xFF;
                                buf[c*6+1] = 0xFF;
                                buf[c*6+2] = 0xFF;
                                buf[c*6+4] = 0xFF;
                                buf[c*6+5] = 0xFF;
                                buf[c*6+6] = 0xFF;
                            } break;
                        }
					} else {
                        switch(cpp) {
                            case 3: {
                                buf[c*3+0] = 0x00;
                                buf[c*3+1] = 0x00;
                                buf[c*3+2] = 0x00;
                            } break;
                            case 4: {
                                buf[c*4+0] = 0x00;
                                buf[c*4+1] = 0x00;
                                buf[c*4+2] = 0x00;
                                buf[c*4+3] = 0x00;
                            } break;
                            case 6: {
                                buf[c*6+0] = 0x00;
                                buf[c*6+1] = 0x00;
                                buf[c*6+2] = 0x00;
                                buf[c*6+3] = 0x00;
                                buf[c*6+4] = 0x00;
                                buf[c*6+5] = 0x00;
                            } break;
                        }
					}
				}
                switch(cpp) {
                    default:
                    case 3: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGB8); break;
                    case 4: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGBW8); break;
                    case 6: lightkraken::Strip::get(strip).setData(buf, l * cpp, Strip::INPUT_dRGB16MSB); break;
                }
			} break;
		}
	};

    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP:
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
        for (size_t c = 0; c < Model::stripN; c++) {
        	effect(c);
        }
	} break;
    case Model::OUTPUT_CONFIG_RGB_STRIP:
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
        for (size_t c = 1; c < Model::stripN; c++) {
        	effect(c);
        }
	} break;
	default: {
	} break;
	} 
    
}

}  // namespace lightkraken {
