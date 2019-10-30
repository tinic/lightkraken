/*
* strip.cpp
*
*  Created on: Sep 5, 2019
*      Author: turo
*/
#include <stdint.h>
#include <string.h>
#include <algorithm>

#include "./strip.h"
#include "./model.h"

namespace lightkraken {

    class manchester_bit_buf {
    public:
            manchester_bit_buf(uint8_t *p) {
                buf = p;
                byte = 0;
                bit_pos = 7;
                byte_pos = 0;
                man_pos = 0;
            }

            __attribute__ ((hot, optimize("O2")))
            void push(uint32_t bits, int32_t count) {
                for (int32_t c = 0; c < count; c++) {
                    for (int32_t d = 0; d < 2; d++) {
                        byte |= ((man_pos & 1) ^ ((bits & (1UL<<31)) ? 1 : 0)) ? (1UL<<bit_pos) : 0;
                        bit_pos--;
                        if (bit_pos < 0) {
                            buf[byte_pos++] = byte;
                            byte = 0;
                            bit_pos = 7;
                        }
                        man_pos++;
                    }
                    bits <<= 1;
                }
            }

            void flush() {
                buf[byte_pos++] = byte;
                byte = 0;
                bit_pos = 7;
            }

            size_t len() {
                return byte_pos;
            }
    private:
        uint8_t *buf;
        uint8_t  byte;
        int32_t	 byte_pos;
        int32_t  bit_pos;
        int32_t  man_pos;
    };

    Strip &Strip::get(size_t index) {
        static Strip strips[lightkraken::Model::stripN];
        static bool init = false;
        if (!init) {
            init = true;
            for (size_t c=0; c<lightkraken::Model::stripN; c++) {
                strips[c].init();
            }
        }
        return strips[index % lightkraken::Model::stripN];
    }

    void Strip::init() {
        memset(comp_buf, 0, sizeof(comp_buf));
        memset(spi_buf, 0, sizeof(spi_buf));
        memset(zero, 0, sizeof(zero));
        transfer_flag = false;
    }

    size_t Strip::getMaxPixelLen() const {
        switch(strip_type) {
            default:
            case WS2812_RGB:
            case SK6812_RGB:
            case GS8208_RGB:
            case TM1804_RGB:
            case TM1829_RGB:
            case UCS1904_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize);
                return padlen * Model::universeN;
            } break;
            case SK6812_RGBW: {
                constexpr size_t pixsize = 4;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize);
                return padlen * Model::universeN;
            } break;
            case LPD8806_RGB:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA102_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize);
                return padlen * Model::universeN;
            } break;
        }
        return 0;
    }
    
    size_t Strip::getComponentsPerPixel() const {
        switch(strip_type) {
            default:
            case WS2812_RGB:
            case SK6812_RGB:
            case GS8208_RGB:
            case TM1804_RGB:
            case UCS1904_RGB:
            case TLS3001_RGB:
            case LPD8806_RGB:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case TM1829_RGB:
            case APA102_RGB: {
                return 3;
            } break;
            case SK6812_RGBW: {
                return 4;
            } break;
        }
    }

    void Strip::setPixelLen(size_t len) {
        switch(strip_type) {
            default:
            case WS2812_RGB:
            case SK6812_RGB:
            case GS8208_RGB:
            case TM1804_RGB:
            case UCS1904_RGB:
            case TLS3001_RGB:
            case LPD8806_RGB:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case TM1829_RGB:
            case APA102_RGB: {
                constexpr size_t pixsize = 3;
                size_t c_len = len * pixsize;
                setLen(c_len);
            } break;
            case SK6812_RGBW: {
                constexpr size_t pixsize = 4;
                size_t c_len = len * pixsize;
                setLen(c_len);
            } break;
        }
    }

    size_t Strip::getMaxLen() const {
        switch(strip_type) {
            default:
            case TLS3001_RGB:
            case LPD8806_RGB:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA102_RGB:
            case WS2812_RGB:
            case SK6812_RGB:
            case GS8208_RGB:
            case TM1804_RGB:
            case TM1829_RGB:
            case UCS1904_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                return padlen * Model::universeN;
            } break;
            case SK6812_RGBW: {
                constexpr size_t pixsize = 4;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                return padlen * Model::universeN;
            } break;
        }
        return 0;
    }

    bool Strip::isBlack() const {
        for (size_t c = 0; c < lightkraken::Model::universeN; c++) {
            if (zero[c]) {
                return false;
            }
        }
        return true;
    }

    void Strip::setLen(size_t len) {
        comp_len = std::min(getMaxLen(), size_t(len));
        memset(&comp_buf[comp_len], 0, sizeof(comp_buf)-comp_len);
    }

    void Strip::setData(const uint8_t *data, size_t len) {
        switch(strip_type) {
            default:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA107_RGB:
            case APA102_RGB:
            case WS2812_RGB:
            case SK6812_RGB:
            case TM1804_RGB:
            case GS8208_RGB:
            case UCS1904_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = Model::universeN * ( size_t(dmxMaxLen / pixsize) * pixsize );
                for (size_t c = 0; c < std::min(len, padlen); c += 3) {
                    comp_buf[c + 0] = data[c+1];
                    comp_buf[c + 1] = data[c+0];
                    comp_buf[c + 2] = data[c+2];
                }
            } break;
            case TM1829_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = Model::universeN * ( size_t(dmxMaxLen / pixsize) * pixsize );
                for (size_t c = 0; c < std::min(len, padlen); c += 3) {
                    comp_buf[c + 0] = data[c+2];
                    comp_buf[c + 1] = data[c+1];
                    comp_buf[c + 2] = data[c+0];
                }
            } break;
            case TLS3001_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = Model::universeN * ( size_t(dmxMaxLen / pixsize) * pixsize );
                for (size_t c = 0; c < std::min(len, padlen); c += 3) {
                    comp_buf[c + 0] = data[c+0];
                    comp_buf[c + 1] = data[c+1];
                    comp_buf[c + 2] = data[c+2];
                }
            } break;
            case SK6812_RGBW: {
                constexpr size_t pixsize = 4;
                constexpr size_t padlen = Model::universeN * ( size_t(dmxMaxLen / pixsize) * pixsize );
                for (size_t c = 0; c < std::min(len, padlen); c += 4) {
                    comp_buf[c + 0] = data[c+0];
                    comp_buf[c + 1] = data[c+1];
                    comp_buf[c + 2] = data[c+2];
                    comp_buf[c + 3] = data[c+3];
                }
            } break;
            case LPD8806_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = Model::universeN * ( size_t(dmxMaxLen / pixsize) * pixsize );
                for (size_t c = 0; c < std::min(len, padlen); c += 3) {
                    comp_buf[c + 0] = data[c+2] >> 1;
                    comp_buf[c + 1] = data[c+0] >> 1;
                    comp_buf[c + 2] = data[c+1] >> 1;
                }
            } break;
        }
    }
    
    bool Strip::isUniverseActive(size_t uniN) const {
        switch(strip_type) {
            default:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA107_RGB:
            case APA102_RGB:
            case WS2812_RGB:
            case SK6812_RGB:
            case TM1804_RGB:
            case GS8208_RGB:
            case TM1829_RGB:
            case TLS3001_RGB:
            case LPD8806_RGB:
            case UCS1904_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                if (uniN * padlen < (comp_len * pixsize )) {
                	return true;
                }
            } break;
            case SK6812_RGBW: {
                constexpr size_t pixsize = 4;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                if (uniN * padlen < (comp_len * pixsize )) {
                	return true;
                }
            } break;
        }
        return false;
    }

    void Strip::setUniverseData(size_t uniN, const uint8_t *data, size_t len) {
        if (uniN >= lightkraken::Model::universeN) {
            return;
        }
        if (!isUniverseActive(uniN)) {
        	return;
        }
        zero[uniN] = 0;
        switch(strip_type) {
            default:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA107_RGB:
            case APA102_RGB:
            case WS2812_RGB:
            case SK6812_RGB:
            case TM1804_RGB:
            case GS8208_RGB:
            case UCS1904_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                for (size_t c = 0; c < std::min(len, padlen); c += 3) {
                    comp_buf[padlen * uniN + c + 0] = data[c+1];
                    comp_buf[padlen * uniN + c + 1] = data[c+0];
                    comp_buf[padlen * uniN + c + 2] = data[c+2];
                    zero[uniN] = data[c+0] | data[c+1] | data[c+2];
                }
            } break;
            case TM1829_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                for (size_t c = 0; c < std::min(len, padlen); c += 3) {
                    comp_buf[padlen * uniN + c + 0] = data[c+2];
                    comp_buf[padlen * uniN + c + 1] = data[c+1];
                    comp_buf[padlen * uniN + c + 2] = data[c+0];
                    zero[uniN] = data[c+0] | data[c+1] | data[c+2];
                }
            } break;
            case TLS3001_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                for (size_t c = 0; c < std::min(len, padlen); c += 3) {
                    comp_buf[padlen * uniN + c + 0] = data[c+0];
                    comp_buf[padlen * uniN + c + 1] = data[c+1];
                    comp_buf[padlen * uniN + c + 2] = data[c+2];
                    zero[uniN] = data[c+0] | data[c+1] | data[c+2];
                }
            } break;
            case SK6812_RGBW: {
                constexpr size_t pixsize = 4;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                for (size_t c = 0; c < std::min(len, padlen); c += 4) {
                    comp_buf[padlen * uniN + c + 0] = data[c+0];
                    comp_buf[padlen * uniN + c + 1] = data[c+1];
                    comp_buf[padlen * uniN + c + 2] = data[c+2];
                    comp_buf[padlen * uniN + c + 3] = data[c+3];
                    zero[uniN] = data[c+0] | data[c+1] | data[c+2];
                }
            } break;
            case LPD8806_RGB: {
                constexpr size_t pixsize = 3;
                constexpr size_t padlen = size_t(dmxMaxLen / pixsize) * pixsize;
                for (size_t c = 0; c < std::min(len, padlen); c += 3) {
                    comp_buf[padlen * uniN + c + 0] = data[c+2] >> 1;
                    comp_buf[padlen * uniN + c + 1] = data[c+0] >> 1;
                    comp_buf[padlen * uniN + c + 2] = data[c+1] >> 1;
                    zero[uniN] = data[c+0] | data[c+1] | data[c+2];
                }
            } break;
        }
    }

    void Strip::transfer() {
        size_t len = 0;
        if (Model::instance().burstMode() &&
            strip_type != TLS3001_RGB) {
            const uint8_t *buf = prepareHead(len);
            if (dmaTransferFunc) dmaTransferFunc((uint8_t *)(buf), uint16_t(len));
            prepareTail();
        } else {
            const uint8_t *buf = prepare(len);
            if (dmaTransferFunc) dmaTransferFunc((uint8_t *)(buf), uint16_t(len));
        }
    }

    const uint8_t *Strip::prepareHead(size_t &len) {
        switch(strip_type) {
            case TLS3001_RGB: {
                return 0;
            } break;
            default:
            case SK6812_RGB:
            case SK6812_RGBW:
            case WS2812_RGB:
            case TM1804_RGB:
            case UCS1904_RGB:
            case TM1829_RGB:
            case GS8208_RGB: {
                len = std::min(sizeof(spi_buf), (comp_len + compLatchLen) * 4);
                ws2812_alike_convert(0, std::min(comp_len + compLatchLen, size_t(burstHeadLen)));
                return spi_buf;
            } break;
            case LPD8806_RGB: {
                len = std::min(sizeof(spi_buf), (comp_len + 1));
                lpd8806_rgb_alike_convert(0, std::min(comp_len + 1, size_t(burstHeadLen)));
                return spi_buf;
            } break;
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA107_RGB:
            case APA102_RGB: {
                size_t ext_len = 32*4 + ( ( comp_len / 2 ) + 7 ) / 8;
                len = std::min(sizeof(spi_buf), (comp_len + ext_len));
                apa102_rgb_alike_convert(0, std::min(comp_len + ext_len, size_t(burstHeadLen)));
                return spi_buf;
            } break;
        }
    }

    void Strip::prepareTail() {
        switch(strip_type) {
            case TLS3001_RGB: {
            } break;
            default:
            case SK6812_RGB:
            case SK6812_RGBW:
            case WS2812_RGB:
            case TM1804_RGB:
            case UCS1904_RGB:
            case TM1829_RGB:
            case GS8208_RGB: {
                ws2812_alike_convert(std::min(comp_len + compLatchLen, size_t(burstHeadLen)), (comp_len + compLatchLen) - 1);
            } break;
            case LPD8806_RGB: {
                lpd8806_rgb_alike_convert(std::min(comp_len + 1, size_t(burstHeadLen)), (comp_len + 1) - 1);
            } break;
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA107_RGB:
            case APA102_RGB: {
                size_t ext_len = 32*4 + ( ( comp_len / 2 ) + 7 ) / 8;
                apa102_rgb_alike_convert(std::min(comp_len + ext_len, size_t(burstHeadLen)), (comp_len + ext_len) - 1);
            } break;
        }
    }

    bool Strip::needsClock() const {
        switch(strip_type) {
            default:
            case TLS3001_RGB: 
            case SK6812_RGB:
            case SK6812_RGBW:
            case WS2812_RGB:
            case TM1804_RGB:
            case UCS1904_RGB:
            case TM1829_RGB:
            case GS8208_RGB: {
                return false;
            } break;
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA107_RGB:
            case APA102_RGB: {
                return true;
            } break;
        }
    }

    const uint8_t *Strip::prepare(size_t &len) {
        switch(strip_type) {
            case TLS3001_RGB: {
                tls3001_alike_convert(len);
                return spi_buf;
            } break;
            default:
            case SK6812_RGB:
            case SK6812_RGBW:
            case WS2812_RGB:
            case TM1804_RGB:
            case UCS1904_RGB:
            case TM1829_RGB:
            case GS8208_RGB: {
                len = std::min(sizeof(spi_buf), (comp_len + compLatchLen) * 4);
                ws2812_alike_convert(0, (comp_len + compLatchLen) - 1);
                return spi_buf;
            } break;
            case LPD8806_RGB: {
                len = std::min(sizeof(spi_buf), (comp_len + 3));
                lpd8806_rgb_alike_convert(0, (comp_len + 3) - 1);
                return spi_buf;
            } break;
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA107_RGB:
            case APA102_RGB: {
                len = std::min(sizeof(spi_buf), (comp_len + 8));
                apa102_rgb_alike_convert(0, (comp_len + 8) - 1);
                return spi_buf;
            } break;
        }
    }

    __attribute__ ((hot, optimize("O3")))
    void Strip::lpd8806_rgb_alike_convert(size_t start, size_t end) {
        uint8_t *dst = spi_buf + start;
        *dst++ = 0x00;
        for (size_t c = std::max(start, size_t(1)); c <= std::min(end, comp_len - 1 + 1); c += 3) {
            *dst++ = 0x80 | comp_buf[c-1+0];
            *dst++ = 0x80 | comp_buf[c-1+1];
            *dst++ = 0x80 | comp_buf[c-1+2];
        }
    }

    __attribute__ ((hot, optimize("O3")))
    void Strip::apa102_rgb_alike_convert(size_t start, size_t end) {
        uint8_t *dst = spi_buf + start;
        // start frame
        size_t head_len = 32;
        for (size_t c = start; c <= std::min(end, size_t(head_len - 1)); c++) {
            *dst++ = 0x00;
            *dst++ = 0x00;
            *dst++ = 0x00;
            *dst++ = 0x00;
        }
        int32_t illum = 0b11100000 | std::min(uint8_t(0x1F), uint8_t((float)0x1f * Model::instance().globIllum()));
        for (size_t c = std::max(start, size_t(head_len)); c <= std::min(end, comp_len - 1 + 1); c += 3) {
            *dst++ = illum;
            *dst++ = comp_buf[c-head_len+0];
            *dst++ = comp_buf[c-head_len+1];
            *dst++ = comp_buf[c-head_len+2];
        }
        // latch words
        for (size_t c = std::max(start, comp_len); c <= end; c++) {
            *dst++ = 0xFF;
            *dst++ = 0xFF;
            *dst++ = 0xFF;
            *dst++ = 0xFF;
        }
    }

    __attribute__ ((hot, optimize("O3")))
    void Strip::ws2812_alike_convert(size_t start, size_t end) {
        uint32_t *dst = (uint32_t *)(spi_buf + start * 4);
        size_t head_len = compLatchLen / 2;
        for (size_t c = start; c <= std::min(end, size_t(head_len - 1)); c++) {
            *dst++ = 0x00;
        }
        for (size_t c = std::max(start, size_t(head_len)); c <= std::min(end, head_len + comp_len - 1); c ++) {
            uint32_t p = uint32_t(comp_buf[c-head_len]);
            *dst++ = 0x88888888UL |
                    (((p >>  4) | (p <<  6) | (p << 16) | (p << 26)) & 0x04040404)|
                    (((p >>  1) | (p <<  9) | (p << 19) | (p << 29)) & 0x40404040);
        }
        for (size_t c = std::max(start, comp_len); c <= end; c++) {
            *dst++ = 0x00;
        }
    }

    __attribute__ ((hot, optimize("O3")))
    void Strip::tls3001_alike_convert(size_t &len) {
        uint8_t *dst = spi_buf;
        uint32_t reset = 0b11111111'11111110'10000000'00000000; // 19 bits
        uint32_t syncw = 0b11111111'11111110'00100000'00000000; // 30 bits
        uint32_t start = 0b11111111'11111110'01000000'00000000; // 19 bits
        manchester_bit_buf buf(dst);
        if (!strip_reset) {
            strip_reset = true;
            buf.push(reset, 19);
            buf.push(0, 4000);
            buf.push(syncw, 30);
            buf.push(0, 12*(comp_len/3));
        } else {
            buf.push(start, 19);
            for (size_t c = 0; c < comp_len; c++) {
                uint32_t p = uint32_t(comp_buf[c]);
                buf.push((p<<19)|(p<<11), 13);
            }
            buf.push(0, 100);
            buf.push(start, 19);
        }
        buf.flush();

        len = buf.len();
    }
}
