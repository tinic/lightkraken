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
#ifndef NETCONF_H
#define NETCONF_H

#include "lwip/init.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"

#define IFNAME0 'L'
#define IFNAME1 'G'

namespace lightkraken {

class NetConf {

public:

    static NetConf &instance();

    void update();
    
#ifndef BOOTLOADER
	bool sendUdpPacket(const ip_addr_t *to, const uint16_t port, const uint8_t *data, uint16_t len);
#endif  // #ifndef BOOTLOADER

    const struct netif *netInterface() const { return &netif; };

private:

    typedef enum 
    { 
        DHCP_START=0,
        DHCP_WAIT_ADDRESS,
        DHCP_ADDRESS_ASSIGNED,
        DHCP_TIMEOUT
    } dhcp_state_enum;

    bool initialized = false;
    void init();

    uint32_t dhcp_fine_timer = 0;
    uint32_t dhcp_coarse_timer = 0;
    dhcp_state_enum dhcp_state = DHCP_START;

    struct netif netif;
    uint32_t tcp_timer = 0;
    uint32_t arp_timer = 0;
    uint32_t config_timer = 0;
    ip_addr_t ip_address = {0};
};

};

#endif /* NETCONF_H */
