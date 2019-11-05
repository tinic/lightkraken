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
#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/err.h"
#include "lwip/netif.h"

namespace lightkraken {

class EthernetIf {
public:
    static EthernetIf &instance();

    static err_t ethernetif_init(struct netif *netif);
    static err_t ethernetif_input(struct netif *netif);

private:

    bool initialized = false;
    void init();

    void low_level_init(struct netif *netif, uint32_t mac_addr);
    
    static err_t low_level_output(struct netif *netif, struct pbuf *p);
    static struct pbuf *low_level_input(struct netif *netif);
    
    uint32_t get_uid0() const;
    uint32_t get_uid1() const;
    uint32_t get_uid2() const;

    uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed) const;
};

}

//err_t ethernetif_init(struct netif *netif);
//err_t ethernetif_input(struct netif *netif);

#endif
