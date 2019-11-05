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
    
    PowerClass powerClass() { readPowerState(); return power_class; }
    
    void setEnetUp() { enet_up = true; }
    bool enetUp() const { return enet_up; }

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
    
    bool enet_up = false;
    
#else  // #ifndef BOOTLOADER
    
    BootloaderStatus status = waiting;
    
#endif  // #ifndef BOOTLOADER

};

};

#endif  // #ifndef STATUS_LED_H
