#ifndef STATUS_LED_H
#define STATUS_LED_H

namespace lightkraken {

class StatusLED {

public:

    static StatusLED &instance();
    
    void update();
    void schedule();

#ifndef BOOTLOADER

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

#else  // #ifndef BOOTLOADER

enum BootloaderStatus {
        waiting,
        erasing,
        flashing,
        done,
        reset
    };

    void setBootloaderStatus(BootloaderStatus s) {
        status = s;
        schedule();
        update();
    }
    
#endif  // #ifdef BOOTLOADER
    
private:
    
    bool scheduled = false;
    bool initialized = false;
    void init();

    void setUserLED(uint8_t r, uint8_t g, uint8_t b);

#ifndef BOOTLOADER
    
    void readPowerState();

    bool bt_state = false;
    bool tpl_state = false;
    bool tph_state = false;
    bool powergood_state = false;
    PowerClass power_class = PSE_TYPE_INVALID;
    
#else  // #ifndef BOOTLOADER
    
    BootloaderStatus status = waiting;
    
#endif  // #ifndef BOOTLOADER

};

};

#endif  // #ifndef STATUS_LED_H
