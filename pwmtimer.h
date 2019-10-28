#ifndef PWM_TIMER_H
#define PWM_TIMER_H

#include <stdint.h>

namespace lightkraken {

class PwmTimer {
public:
    constexpr static uint16_t pwmPeriod = 0x3fff;

    virtual void setPulse(uint16_t pulse) = 0;
protected:
    bool initialized = false;
    virtual void init() = 0;
};

class PwmTimer0 : public PwmTimer {
public:
    static PwmTimer &instance();
    virtual void setPulse(uint16_t pulse);
private:
    virtual void init();
};

class PwmTimer1 : public PwmTimer {
public:
    static PwmTimer &instance();
    virtual void setPulse(uint16_t pulse);
private:
    virtual void init();
};

class PwmTimer2 : public PwmTimer {
public:
    static PwmTimer &instance();
    virtual void setPulse(uint16_t pulse);
private:
    virtual void init();
};

class PwmTimer3 : public PwmTimer {
public:
    static PwmTimer &instance();
    virtual void setPulse(uint16_t pulse);
private:
    virtual void init();
};

class PwmTimer5 : public PwmTimer {
public:
    static PwmTimer &instance();
    virtual void setPulse(uint16_t pulse);
private:
    virtual void init();
};

class PwmTimer6 : public PwmTimer {
public:
    static PwmTimer &instance();
    virtual void setPulse(uint16_t pulse);
private:
    virtual void init();
};


} //  namespace lightkraken {

#endif  // #ifndef PWM_TIMER_H
