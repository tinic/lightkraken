#ifndef CONTROL_H
#define CONTROL_H

#include <functional>

namespace lightkraken {

class Control {
public:
    static Control &instance();

    void setUniverseOutputData(uint16_t universe, const uint8_t *data, size_t len, bool nodriver = false);

    void sync();

    void setEnableSyncMode(bool state) { syncMode = state; }
	void interateAllActiveUniverses(std::function<void (uint16_t universe)> callback);

private:

	bool syncMode = false;
    void setUniverseOutputDataForDriver(size_t channels, size_t components, uint16_t uni, const uint8_t *data, size_t len);
    bool initialized = false;
    void init();
};

}
#endif  // #ifndef CONTROL_H
