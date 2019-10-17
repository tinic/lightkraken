#ifndef STATUS_LED_H
#define STATUS_LED_H

namespace lightguy {

class StatusLED {

public:

	static StatusLED &instance();

    void update();

private:
	bool initialized = false;
	void init();

    void readPowerState();
	void setUserLED(uint8_t r, uint8_t g, uint8_t b);

  	bool bt_state = false;
	bool tpl_state = false;
	bool tph_state = false;
	bool powergood_state = false;

};

};

#endif  // #ifndef STATUS_LED_H
