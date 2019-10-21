#ifndef SYSTICK_H
#define SYSTICK_H

namespace lightguy {

class Systick {
public:
    static Systick &instance();
    
    uint32_t systemTime() const { return system_time; }
    
    void handler();
    
	void scheduleReset(uint32_t countdown = 1000) { nvic_reset_delay = countdown; }
	
private:

    bool initialized = false;
    void init();

    uint32_t system_time = 0;
    uint32_t nvic_reset_delay = 0;
};

}
#endif  // #ifndef SYSTICK_H
