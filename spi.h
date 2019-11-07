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
#ifndef SPI_H
#define SPI_H

namespace lightkraken {

class SPI {
protected:
    bool initialized = false;
    const uint8_t *cbuf = 0;
    bool sclk = false;
    size_t clen = 0;
    bool active = false;
    bool scheduled = false;
};

class SPI_0 : public SPI {
public:
    static SPI_0 &instance();

    void transfer(const uint8_t *buf, size_t len, bool wantsSCLK);
    void update();
    bool busy() const;
private:

    void init();
    void dma_setup();
};

class SPI_2 : public SPI {
public:
    static SPI_2 &instance();

    void transfer(const uint8_t *buf, size_t len, bool wantsSCLK);
    void update();
    bool busy() const;
private:

    void init();
    void dma_setup();
};

}

#endif  // #ifndef SPI_H
