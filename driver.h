/*
* driver.h
*
*  Created on: Sep 17, 2019
*      Author: Tinic Uro
*/

#ifndef LIGHTGUY_DRIVER_H_
#define LIGHTGUY_DRIVER_H_

#include <stdint.h>
#include <memory.h>

namespace lightguy {

class Driver {
public:
    static Driver &instance();

    void setRGBW8CIE(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void setRGB8CIE(uint8_t r, uint8_t g, uint8_t b);

    void setRGBW16(uint16_t r, uint16_t g, uint16_t b, uint16_t w);
    void setRGB16(uint16_t r, uint16_t g, uint16_t b);

private:

    void setPulse(size_t idx, uint16_t pulse);

    bool initialized = false;
    void init();
};

};

#endif /* LIGHTGUY_DRIVER_H_ */
