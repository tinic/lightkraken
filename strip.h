/*
* strip.h
*
*  Created on: Sep 5, 2019
*      Author: turo
*/

#ifndef STRIP_H_
#define STRIP_H_

#include <stdint.h>
#include <memory.h>
#include <functional>

#include "model.h"

namespace lightguy {

    class Strip {
    public:

        enum Type {
            WS2812_RGB,
            SK6812_RGB,
            TM1804_RGB,
            UCS1904_RGB,
            GS8208_RGB,
            SK6812_RGBW,
            APA102_RGB,
            APA107_RGB,
            P9813_RGB,
            SK9822_RGB,
            HDS107S_RGB,
            LPD8806_RGB,
            TLS3001_RGB, // TODO
            TM1829_RGB,
            TYPE_COUNT
        };

        static constexpr size_t dmxMaxLen = 512;
        static constexpr size_t compMaxLen = (dmxMaxLen*lightguy::Model::universeN);
        static constexpr size_t compLatchLen = 64;
        static constexpr size_t spiMaxLen = (compMaxLen+compLatchLen)*sizeof(uint32_t);
        static constexpr size_t burstHeadLen = 64;

        static Strip &get(size_t index);

        bool isBlack() const;
        bool needsClock() const;

        void setStripType(Type type) { strip_type = type; }

        void setPixelLen(size_t len);
        size_t getMaxPixelLen() const;

        void setUniverseData(size_t N, const uint8_t *data, size_t len);
        void setData(const uint8_t *data, size_t len);

        void transfer();

        std::function<void (const uint8_t *data, size_t len)> dmaTransferFunc;
        std::function<bool ()> dmaBusyFunc;

        void setPendingTransferFlag() { transfer_flag = true; }
        bool pendingTransferFlag() { if (transfer_flag) { transfer_flag = false; return true; } return false; }

    private:
        void init();

        void setLen(size_t len);
        size_t getMaxLen() const;

        const uint8_t *prepareHead(size_t &len);
        void prepareTail();
        const uint8_t *prepare(size_t &len);

        void lpd8806_rgb_alike_convert(size_t start, size_t end);
        void apa102_rgb_alike_convert(size_t start, size_t end);
        void ws2812_alike_convert(size_t start, size_t end);
        void tls3001_alike_convert(size_t &len);

        bool transfer_flag;
        bool strip_reset = false;
        uint8_t zero[lightguy::Model::universeN];
        Type strip_type = WS2812_RGB;
        size_t comp_len = 0;
        uint8_t comp_buf[compMaxLen];
        uint8_t spi_buf[spiMaxLen];
    };

}

#endif /* STRIP_H_ */
