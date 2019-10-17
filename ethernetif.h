#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/err.h"
#include "lwip/netif.h"

namespace lightguy {

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
