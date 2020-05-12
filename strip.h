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
#ifndef STRIP_H_
#define STRIP_H_

#include <stdint.h>
#include <string.h>
#include <functional>
#include <array>

#include "./model.h"

namespace lightkraken {
    
    class Strip {
    public:

        struct DitherPixel {
            uint16_t value;
            int8_t   error;
        } __attribute__((packed));

    	enum InputType {
    		INPUT_dRGB8,
    		INPUT_dRGBW8,
    		INPUT_sRGB8,
    		INPUT_sRGBW8,

    		INPUT_dRGB16MSB,
    		INPUT_dRGBW16MSB,
    		INPUT_dRGB16LSB,
    		INPUT_dRGBW16LSB,

            INPUT_TYPE_COUNT
    	};

    	enum NativeType {
    		NATIVE_RGB8,
    		NATIVE_RGBW8,
            
    		NATIVE_RGB16,

            NATIVE_TYPE_COUNT
    	};

        enum OutputType {
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
            WS2816_RGB,
            OUTPUT_TYPE_COUNT
        };

        static constexpr size_t dmxMaxLen = 512;
        static constexpr size_t bytesMaxLen = (dmxMaxLen*lightkraken::Model::universeN);
        static constexpr size_t bytesLatchLen = 64;
        static constexpr size_t spiMaxLen = (bytesMaxLen*sizeof(uint32_t)+bytesLatchLen*sizeof(uint32_t));
        static constexpr size_t burstHeadLen = 128;

        static Strip &get(size_t index);

        bool needsClock() const;

        void setStripType(OutputType type) { output_type = type; }
        void setRGBColorSpace(const RGBColorSpace &colorSpace);
        void setCompLimit(float value) { comp_limit = value; };
        void setGlobIllum(float value) { glob_illum = value; };

        void setPixelLen(size_t len);
        size_t getPixelLen() const;
        size_t getMaxPixelLen() const;
        size_t getBytesPerPixel() const;

	    NativeType nativeType() const;

        void setUniverseData(size_t N, const uint8_t *data, size_t len, InputType input_type);
        void setData(const uint8_t *data, size_t len, InputType input_type);
        bool isUniverseActive(size_t uniN, InputType input_type) const;

        void transfer();

        std::function<void (const uint8_t *data, size_t len)> dmaTransferFunc;
        std::function<bool ()> dmaBusyFunc;

        void setPendingTransferFlag() { transfer_flag = true; }
        bool pendingTransferFlag() { if (transfer_flag) { transfer_flag = false; return true; } return false; }

    private:
        bool use32Bit();
        
        void init();

        void setBytesLen(size_t len);
        size_t getMaxBytesLen() const;
	    size_t getBytesPerInputPixel(InputType input_type) const;
	    size_t getComponentsPerInputPixel(InputType input_type) const;
	    size_t getComponentBytes(InputType input_type) const;

        const uint8_t *prepareHead(size_t &len);
        void prepareTail();
        const uint8_t *prepare(size_t &len);

        void lpd8806_rgb_alike_convert(size_t start, size_t end);
        void apa102_rgb_alike_convert(size_t start, size_t end);
        void ws2812_alike_convert(size_t start, size_t end);
        void tls3001_alike_convert(size_t &len);

        bool transfer_flag;
        bool strip_reset = false;
        OutputType output_type = WS2812_RGB;
        size_t bytes_len = 0;
        float comp_limit = 1.0f;
        float glob_illum = 1.0f;
        std::array<uint8_t, bytesMaxLen> comp_buf;
        std::array<uint8_t, spiMaxLen> spi_buf;
    };

}

#endif /* STRIP_H_ */
