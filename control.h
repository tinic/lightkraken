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
#ifndef CONTROL_H
#define CONTROL_H

#include <functional>

#include "./spi.h"
#include "./model.h"

namespace lightkraken {

class Control {
public:
    static Control &instance();

    void setUniverseOutputData(uint16_t universe, const uint8_t *data, size_t len, bool nodriver = false);

    void sync();
    void syncFromInterrupt(const SPI &spi);
    void update();

    void setEnableSyncMode(bool state) { syncMode = state; }
    bool syncModeEnabled() const { return syncMode; }
	void interateAllActiveArtnetUniverses(std::function<void (uint16_t universe)> callback);
    void collectAllActiveArtnetUniverses(std::array<uint16_t, Model::maxUniverses> &universes, size_t &universeCount);
	void collectAllActiveE131Universes(std::array<uint16_t, Model::maxUniverses> &universes, size_t &universeCount);

    void setDataReceived() { data_received = true; }
    bool dataReceived() const { return data_received; }
    void scheduleColor() { color_scheduled = true; }
    void setColor();
    
private:

    bool color_scheduled = false;
    bool data_received = false;
	bool syncMode = false;
    void setUniverseOutputDataForDriver(size_t channels, size_t components, uint16_t uni, const uint8_t *data, size_t len);
    bool initialized = false;
    void init();
};

}
#endif  // #ifndef CONTROL_H
