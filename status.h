#ifndef STATUS_LED_H
#define STATUS_LED_H

namespace lightguy {

class StatusLED {

public:

	static StatusLED &instance();

    void update();
    void schedule() { scheduled = true; }

    enum PowerClass {
        PSE_TYPE_INVALID        = 0b011,
        PSE_TYPE_POWER_BAD      = 0b001,
        
        PSE_TYPE_1_2_CLASS_0_3  = 0b111,
        PSE_TYPE_2_CLASS_4      = 0b101,
        PSE_TYPE_3_4_CLASS_0_3  = 0b110,
        PSE_TYPE_3_4_CLASS_4    = 0b100,
        PSE_TYPE_3_4_CLASS_5_6  = 0b010,
        PSE_TYPE_4_CLASS_7_8    = 0b000,
    };
    
    PowerClass powerClass() const { return power_class; }
    
private:
	bool initialized = false;
	void init();

    void readPowerState();
	void setUserLED(uint8_t r, uint8_t g, uint8_t b);

  	bool bt_state = false;
	bool tpl_state = false;
	bool tph_state = false;
	bool powergood_state = false;
    bool scheduled = false;
    PowerClass power_class = PSE_TYPE_INVALID;

};

};

#endif  // #ifndef STATUS_LED_H
