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
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <vector>

#include "./strip.h"
#include "./model.h"
#include "./color.h"
#include "./perf.h"

namespace lightkraken {

    static ColorSpaceConverter converter;

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
        transfer_flag = false;
        RGBColorSpace rgbSpace;
        rgbSpace.setLED();
        converter.setRGBColorSpace(rgbSpace);
    }

    void Strip::setRGBColorSpace(const RGBColorSpace &colorSpace) {
        converter.setRGBColorSpace(colorSpace);
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

    void Strip::setLen(size_t len) {
        comp_len = std::min(getMaxLen(), size_t(len));
        memset(&comp_buf[comp_len], 0, sizeof(comp_buf)-comp_len);
    }
    
    bool Strip::use32Bit() {
        return convertsrgb && dither && (Model::instance().outputMode() == Model::MODE_INTERRUPT);
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

    void Strip::setData(const uint8_t *data, size_t len) {
		PerfMeasure perf(PerfMeasure::SLOT_STRIP_COPY);

        auto transfer = [=] (const std::vector<int> &order) {
            size_t padlen = Model::universeN * ( size_t(dmxMaxLen / order.size()) * order.size() );
            if (use32Bit()) {
                if (convertsrgb) {
                    converter.sRGB8toLED16(std::min(len, padlen), data, reinterpret_cast<uint32_t *>(&comp_buf[0]), order[0], order[1], order[2], order.size()); 
                    if (order.size() > 3) {
                        converter.sRGB8TransfertoLED16Transfer(std::min(len, padlen), data, reinterpret_cast<uint32_t *>(&comp_buf[0]), order[3], 3, order.size());
                    }
                } else {
                    uint32_t *buf = reinterpret_cast<uint32_t *>(&comp_buf[0]);
                    for (size_t c = 0; c < std::min(len, padlen); c += order.size()) {
                        for (size_t d = 0; d < order.size(); d++) {
                            buf[c + d] = data[c + order[d]] << 8;
                        }
                    }
                }
            } else {
                if (convertsrgb) {
                    converter.sRGB8toLED8(std::min(len, padlen), data, &comp_buf[0], order[0], order[1], order[2], order.size()); 
                    if (order.size() > 3) {
                        converter.sRGB8TransfertoLED8Transfer(std::min(len, padlen), data, &comp_buf[0], order[3], 3, order.size());
                    }
                } else {
                    uint8_t *buf = reinterpret_cast<uint8_t *>(&comp_buf[0]);
                    for (size_t c = 0; c < std::min(len, padlen); c += order.size()) {
                        for (size_t d = 0; d < order.size(); d++) {
                            buf[c + d] = data[c + order[d]];
                        }
                    }
                }
            }
        };

        switch(strip_type) {
            default:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case WS2812_RGB:
            case SK6812_RGB:
            case TM1804_RGB:
            case GS8208_RGB:
            case UCS1904_RGB: {
                const std::vector<int> order = { 1, 0, 2 };
                transfer(order);
            } break;
            case APA107_RGB:
            case APA102_RGB:
            case TM1829_RGB: {
                const std::vector<int> order = { 2, 1, 0 };
                transfer(order);
            } break;
            case TLS3001_RGB: {
                const std::vector<int> order = { 0, 1, 2 };
                transfer(order);
            } break;
            case SK6812_RGBW: {
                const std::vector<int> order = { 0, 1, 2, 3 };
                transfer(order);
            } break;
            case LPD8806_RGB: {
                const std::vector<int> order = { 2, 0, 1 };
                transfer(order);
            } break;
        }

    }
    
    void Strip::setUniverseData(size_t uniN, const uint8_t *data, size_t len) {
		PerfMeasure perf(PerfMeasure::SLOT_STRIP_COPY);

        if (uniN >= lightkraken::Model::universeN) {
            return;
        }

        if (!isUniverseActive(uniN)) {
        	return;
        }
        
        auto transfer = [=] (const std::vector<int> &order) {
            size_t padlen = size_t(dmxMaxLen / order.size()) * order.size();
            if (use32Bit()) {
                if (convertsrgb) {
                    converter.sRGB8toLED16(std::min(len, padlen), data, reinterpret_cast<uint32_t *>(&comp_buf[padlen * uniN]), order[0], order[1], order[2], order.size()); 
                    if (order.size() > 3) {
                        converter.sRGB8TransfertoLED16Transfer(std::min(len, padlen), data, reinterpret_cast<uint32_t *>(&comp_buf[padlen * uniN]), order[3], 3, order.size());
                    }
                } else {
                    uint32_t *buf = reinterpret_cast<uint32_t *>(&comp_buf[padlen * uniN]);
                    for (size_t c = 0; c < std::min(len, padlen); c += order.size()) {
                        for (size_t d = 0; d < order.size(); d++) {
                            buf[c + d] = data[c + order[d]] << 8;
                        }
                    }
                }
            } else {
                if (convertsrgb) {
                    converter.sRGB8toLED8(std::min(len, padlen), data, &comp_buf[padlen * uniN], order[0], order[1], order[2], order.size()); 
                    if (order.size() > 3) {
                        converter.sRGB8TransfertoLED8Transfer(std::min(len, padlen), data, &comp_buf[padlen * uniN], order[3], 3, order.size());
                    }
                } else {
                    uint8_t *buf = reinterpret_cast<uint8_t *>(&comp_buf[padlen * uniN]);
                    for (size_t c = 0; c < std::min(len, padlen); c += order.size()) {
                        for (size_t d = 0; d < order.size(); d++) {
                            buf[c + d] = data[c + order[d]];
                        }
                    }
                }
            }
        };
        
        switch(strip_type) {
            default:
            case SK9822_RGB:
            case HDS107S_RGB:
            case P9813_RGB:
            case APA107_RGB:
            case WS2812_RGB:
            case SK6812_RGB:
            case TM1804_RGB:
            case GS8208_RGB:
            case UCS1904_RGB: {
                const std::vector<int> order = { 1, 0, 2 };
                transfer(order);
            } break;
            case APA102_RGB:
            case TM1829_RGB: {
                const std::vector<int> order = { 2, 1, 0 };
                transfer(order);
            } break;
            case TLS3001_RGB: {
                const std::vector<int> order = { 0, 1, 2 };
                transfer(order);
            } break;
            case SK6812_RGBW: {
                const std::vector<int> order = { 0, 1, 2, 3 };
                transfer(order);
            } break;
            case LPD8806_RGB: {
                const std::vector<int> order = { 2, 0, 1 };
                transfer(order);
            } break;
        }
    }

    void Strip::transfer() {
		PerfMeasure perf(PerfMeasure::SLOT_STRIP_TRANFER);
        size_t len = 0;
        if (Model::instance().burstMode() &&
            strip_type != TLS3001_RGB) {
            const uint8_t *buf = prepareHead(len);
            if (dmaTransferFunc) {
                dmaTransferFunc((uint8_t *)(buf), uint16_t(len));
            }
            prepareTail();
        } else {
            const uint8_t *buf = prepare(len);
            if (dmaTransferFunc) {
                dmaTransferFunc((uint8_t *)(buf), uint16_t(len));
            }
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
	            size_t out_len = comp_len + comp_len / 3;
                size_t ext_len = 32 + ( ( comp_len / 2 ) + 7 ) / 8;
                len = std::min(sizeof(spi_buf), (out_len + ext_len));
                apa102_rgb_alike_convert(0, std::min(out_len + ext_len, size_t(burstHeadLen)));
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
	            size_t out_len = comp_len + comp_len / 3;
                size_t ext_len = 32 + ( ( comp_len / 2 ) + 7 ) / 8;
                apa102_rgb_alike_convert(std::min(out_len + ext_len, size_t(burstHeadLen)), (out_len + ext_len) - 1);
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
	            size_t out_len = comp_len + comp_len / 3;
                size_t ext_len = 32 + ( ( out_len / 2 ) + 7 ) / 8;
                len = std::min(sizeof(spi_buf), (out_len + ext_len));
                apa102_rgb_alike_convert(0, (out_len + ext_len) - 1);
                return spi_buf;
            } break;
        }
    }

    __attribute__ ((hot, optimize("O3")))
    void Strip::lpd8806_rgb_alike_convert(size_t start, size_t end) {
        uint8_t *dst = spi_buf + start;
        *dst++ = 0x00;
        if (use32Bit()) {
            uint32_t *comp_buf32 = reinterpret_cast<uint32_t *>(comp_buf);
            for (size_t c = std::max(start, size_t(1)); c <= std::min(end, 1 + comp_len - 1); c++) {
                *dst++ = 0x80 | ((comp_buf32[c-1] >> 9) & 0x7F);
            }
        } else {
            for (size_t c = std::max(start, size_t(1)); c <= std::min(end, 1 + comp_len - 1); c++) {
                *dst++ = 0x80 | (comp_buf[c-1] >> 1);
            }
        }
    }

    __attribute__ ((hot, optimize("O3")))
    void Strip::apa102_rgb_alike_convert(size_t start, size_t end) {

		// Align to 4 bytes
        start &= ~3;

        uint8_t *dst = spi_buf + start;
        size_t out_len = comp_len + ( comp_len / 3 );

        // start frame
        size_t head_len = 32;
        for (size_t c = start; c <= std::min(end, size_t(head_len - 1)); c++) {
            *dst++ = 0x00;
        }

        size_t offset = 0;
		// adjust offset
		for (size_t c = head_len; c < start; c += 4, offset += 3) { }
		
		size_t loop_start = std::max(start, size_t(head_len));
		size_t loop_end = std::min(end, head_len + out_len - 1);
		
        uint8_t illum = 0b11100000 | std::min(uint8_t(0x1F), uint8_t((float)0x1f * Model::instance().globIllum()));
        if (use32Bit()) {
            uint32_t *comp_buf32 = reinterpret_cast<uint32_t *>(comp_buf);
            if (dither && Model::instance().outputMode() == Model::MODE_INTERRUPT) {
                for (size_t c = loop_start; c <= loop_end; c += 4, offset += 3) {
                    *dst++ = illum;
                    for (size_t d = 0; d < 3; d++) {
                        int32_t v = int32_t(comp_buf32[offset+d] & 0xFFFF) + int32_t(int16_t(comp_buf32[offset+d] >> 16));
                        int32_t p = v >> 8;
                        comp_buf32[offset+d] = uint32_t(int32_t(comp_buf32[offset+d] & 0xFFFF) | (int32_t((v - (p << 8))) << 16));
                        *dst++ = p & 0xFF;
                    }
                }
            } else {
                for (size_t c = loop_start; c <= loop_end; c += 4, offset += 3) {
                    *dst++ = illum;
                    *dst++ = (comp_buf32[offset+0] >> 8) & 0xFF;
                    *dst++ = (comp_buf32[offset+1] >> 8) & 0xFF;
                    *dst++ = (comp_buf32[offset+2] >> 8) & 0xFF;
                }
            }
        } else {
            for (size_t c = loop_start; c <= loop_end; c += 4, offset += 3) {
                *dst++ = illum;
                *dst++ = comp_buf[offset+0];
                *dst++ = comp_buf[offset+1];
                *dst++ = comp_buf[offset+2];
            }
        }
        // latch words
        for (size_t c = std::max(start, head_len + out_len); c <= end; c++) {
            *dst++ = 0x00;
            *dst++ = 0x00;
            *dst++ = 0x00;
            *dst++ = 0x00;
        }
    }

    __attribute__ ((hot, optimize("O3")))
    void Strip::ws2812_alike_convert(size_t start, size_t end) {
        uint32_t *dst = (uint32_t *)(spi_buf + start * 4);
        size_t head_len = compLatchLen / 2;
        for (size_t c = start; c <= std::min(end, size_t(head_len - 1)); c++) {
            *dst++ = 0x00;
        }
        auto conv_to_one_wire = [=] (uint32_t p) mutable {
			*dst++ = 0x88888888UL |
					(((p >>  4) | (p <<  6) | (p << 16) | (p << 26)) & 0x04040404)|
					(((p >>  1) | (p <<  9) | (p << 19) | (p << 29)) & 0x40404040);
        };
        if (use32Bit()) {
            uint32_t *comp_buf32 = reinterpret_cast<uint32_t *>(comp_buf);
            if (dither && Model::instance().outputMode() == Model::MODE_INTERRUPT) {
                for (size_t c = std::max(start, size_t(head_len)); c <= std::min(end, head_len + comp_len - 1); c++) {
                    int32_t v = int32_t(comp_buf32[c-head_len] & 0xFFFF) + int32_t(int16_t(comp_buf32[c-head_len] >> 16));
                    int32_t p = v >> 8;
                    comp_buf32[c-head_len] = uint32_t(int32_t(comp_buf32[c-head_len] & 0xFFFF) | (int32_t((v - (p << 8))) << 16));
                    conv_to_one_wire(p);
                }
            } else {
                for (size_t c = std::max(start, size_t(head_len)); c <= std::min(end, head_len + comp_len - 1); c++) {
                    conv_to_one_wire(( uint32_t(comp_buf32[c-head_len]) >> 8 ) & 0xFF);
                }
            }
        } else {
			for (size_t c = std::max(start, size_t(head_len)); c <= std::min(end, head_len + comp_len - 1); c++) {
				conv_to_one_wire(uint32_t(comp_buf[c-head_len]));
			}
		}
        for (size_t c = std::max(start, head_len + comp_len); c <= end; c++) {
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
            if (use32Bit()) {
                for (size_t c = 0; c < comp_len; c++) {
                    uint32_t p = uint32_t((comp_buf[c] >> 8) & 0xFF);
                    buf.push((p<<19)|(p<<11), 13);
                }
            } else {
                for (size_t c = 0; c < comp_len; c++) {
                    uint32_t p = uint32_t(comp_buf[c]);
                    buf.push((p<<19)|(p<<11), 13);
                }
            }
            buf.push(0, 100);
            buf.push(start, 19);
        }
        buf.flush();

        len = buf.len();
    }
}
