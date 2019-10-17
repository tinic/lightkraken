#ifndef CONTROL_H
#define CONTROL_H

namespace lightguy {

class Control {
public:
	static Control &instance();

	void setUniverseOutputData(uint16_t universe, const uint8_t *data, size_t len);
	void setOutputData(uint8_t channel, const uint8_t *data, size_t len);
	void transferNow();

private:
	bool initialized = false;
	void init();
};

}
#endif  // #ifndef CONTROL_H
