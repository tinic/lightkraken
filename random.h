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
#ifndef _RANDOM_H_
#define _RANDOM_H_

namespace lightkraken {

class PseudoRandom {
public:
    static PseudoRandom &instance();
    
    int32_t get(int32_t lower, int32_t upper) {
        return (static_cast<int32_t>(get()) % (upper-lower)) + lower;
    }

private:
    bool initialized = false;
    void init();

    void set_seed(uint32_t seed);
    uint32_t get();

    uint32_t a; 
    uint32_t b; 
    uint32_t c; 
    uint32_t d; 
};

}

#endif  // #ifndef _RANDOM_H_

