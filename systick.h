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
#ifndef SYSTICK_H
#define SYSTICK_H

namespace lightkraken {

class Systick {
public:
    static Systick &instance();
    
    uint32_t systemTime() const { return system_time; }
	uint64_t systemTick();
 
    void handler();
    
	void scheduleReset(int32_t countdown = 2000, bool bootloader = false) { 
        nvic_reset_delay = countdown; 
        bootloader_after_reset = bootloader;
    }
	
	void scheduleApply() {
        apply_scheduled = true;
    }
	
private:

    bool initialized = false;
    void init();

    uint32_t system_time = 0;
    bool bootloader_after_reset = false;
    int32_t nvic_reset_delay = 0;
    bool apply_scheduled = false;
};

}
#endif  // #ifndef SYSTICK_H
