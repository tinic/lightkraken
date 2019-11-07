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

void Control::syncFromInterrupt(const SPI &spi) {
	if (Model::instance().outputMode() != Model::MODE_INTERRUPT) {
		return;
	}
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
    	if (&spi == &SPI_0::instance()) {
			lightkraken::Strip::get(0).transfer();
		}
    	if (&spi == &SPI_2::instance()) {
			lightkraken::Strip::get(1).transfer();
		}
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
    	if (&spi == &SPI_0::instance()) {
			lightkraken::Strip::get(0).transfer();
		}
    	if (&spi == &SPI_2::instance()) {
			lightkraken::Strip::get(1).transfer();
		}
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
    	if (&spi == &SPI_2::instance()) {
			lightkraken::Strip::get(1).transfer();
		}
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
    	if (&spi == &SPI_2::instance()) {
			lightkraken::Strip::get(1).transfer();
		}
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
    } break;
    }
}

void Control::sync() {
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
		if (Model::instance().outputMode() == Model::MODE_MAIN_LOOP) {
			for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
				lightkraken::Strip::get(c).transfer();
			}
		}
	} break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
		Driver::instance().sync(0);
		if (Model::instance().outputMode() == Model::MODE_MAIN_LOOP) {
			for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
				lightkraken::Strip::get(c).transfer();
			}
		}
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
		Driver::instance().sync(0);
		if (Model::instance().outputMode() == Model::MODE_MAIN_LOOP) {
			for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
				lightkraken::Strip::get(c).transfer();
			}
		}
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
		Driver::instance().sync(0);
		if (Model::instance().outputMode() == Model::MODE_MAIN_LOOP) {
			for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
				lightkraken::Strip::get(c).transfer();
			}
		}
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
		for (size_t c = 0; c < Model::analogN; c++) {
			Driver::instance().sync(c);
		}
	} break;
	}
}

void Control::interateAllActiveUniverses(std::function<void (uint16_t universe)> callback) {
	static constexpr size_t maxUniverses = Model::stripN * Model::universeN + Model::analogN * Model::analogCompN;
	class UniqueCollector {
	public:
		UniqueCollector() {
			memset(&collected_universes[0], 0xFF, sizeof(collected_universes));
		}
	
		void maybeAcquire(uint16_t universe) {
			for (size_t c = 0; c < maxUniverses; c++) {
				if (collected_universes[c] == universe) {
					return;
				}
				if (collected_universes[c] == 0xFFFF) {
				 	collected_universes[c] = universe;
				 	return;
				}
			}
		}

		void showcase(std::function<void (uint16_t universe)> callback) {
			for (size_t c = 0; c < maxUniverses; c++) {
				callback(collected_universes[c]);
				if (collected_universes[c] == 0xFFFF) {
					return;
				}
			}
		}
	private:
		uint16_t collected_universes[maxUniverses];
	} uniqueCollector;

	switch(Model::instance().outputConfig()) {
	case Model::OUTPUT_CONFIG_DUAL_STRIP: {
		for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
			for (size_t d = 0; d < Model::universeN; d++) {
				if (Strip::get(c).isUniverseActive(d)) {
					uniqueCollector.maybeAcquire(Model::instance().universeStrip(c,d));
				}
			}
		}
	} break;
	case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[0].universe);
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[1].universe);
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[2].universe);
		for (size_t c = 0; c < lightkraken::Model::stripN; c++) {
			for (size_t d = 0; d < Model::universeN; d++) {
				if (Strip::get(c).isUniverseActive(d)) {
					uniqueCollector.maybeAcquire(Model::instance().universeStrip(c,d));
				}
			}
		}
	} break;
	case Model::OUTPUT_CONFIG_RGB_STRIP: {
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[0].universe);
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[1].universe);
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[2].universe);
		for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
			for (size_t d = 0; d < Model::universeN; d++) {
				if (Strip::get(c).isUniverseActive(d)) {
					uniqueCollector.maybeAcquire(Model::instance().universeStrip(c,d));
				}
			}
		}
	} break;
	case Model::OUTPUT_CONFIG_RGBW_STRIP: {
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[0].universe);
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[1].universe);
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[2].universe);
		uniqueCollector.maybeAcquire(Model::instance().analogConfig(0).components[3].universe);
		for (size_t c = 1; c < lightkraken::Model::stripN; c++) {
			for (size_t d = 0; d < Model::universeN; d++) {
				if (Strip::get(c).isUniverseActive(d)) {
					uniqueCollector.maybeAcquire(Model::instance().universeStrip(c,d));
				}
			}
		}
	} break;
	case Model::OUTPUT_CONFIG_RGB_RGB: {
		for (size_t c = 0; c < Model::analogN; c++) {
			uniqueCollector.maybeAcquire(Model::instance().analogConfig(c).components[0].universe);
			uniqueCollector.maybeAcquire(Model::instance().analogConfig(c).components[1].universe);
			uniqueCollector.maybeAcquire(Model::instance().analogConfig(c).components[2].universe);
		}
	} break;
	}
	
	uniqueCollector.showcase(callback);
}

void Control::setUniverseOutputDataForDriver(size_t terminals, size_t components, uint16_t uni, const uint8_t *data, size_t len) {
    rgbww rgb[Driver::terminalN];
    for (size_t c = 0; c < terminals; c++) {
        rgb[c] = Driver::instance().srgbwwCIE(c);
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
        Driver::instance().setsRGBWWCIE(c,rgb[c]);
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
            if (set && !syncMode && Model::instance().outputMode() == Model::MODE_MAIN_LOOP) {
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
            if (set && !syncMode && Model::instance().outputMode() == Model::MODE_MAIN_LOOP) {
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
            if (set && !syncMode && Model::instance().outputMode() == Model::MODE_MAIN_LOOP) {
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
            if (set && !syncMode && Model::instance().outputMode() == Model::MODE_MAIN_LOOP) {
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

void Control::update() {
	if (lightkraken::Model::instance().outputMode() == lightkraken::Model::MODE_MAIN_LOOP) {
		lightkraken::SPI_2::instance().update();
		lightkraken::SPI_0::instance().update();
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

}  // namespace lightkraken {
