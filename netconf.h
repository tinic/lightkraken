#ifndef NETCONF_H
#define NETCONF_H

#define IFNAME0 'G'
#define IFNAME1 'D'

#define MAC_ADDR0   	0x1E
#define MAC_ADDR1   	0xD5
#define MAC_ADDR2   	0x10
#define MAC_ADDR3  		0x10
#define MAC_ADDR4   	0x10
#define MAC_ADDR5   	0x10

#define IP_ADDR0 		192
#define IP_ADDR1 		168
#define IP_ADDR2 		  1
#define IP_ADDR3 		100

#define NETMASK_ADDR0 	255
#define NETMASK_ADDR1 	255
#define NETMASK_ADDR2 	255
#define NETMASK_ADDR3 	  0

#define GW_ADDR0 		192
#define GW_ADDR1 		168
#define GW_ADDR2 		  1
#define GW_ADDR3		  1

void lwip_dhcp_process_handle(void);
void lwip_stack_init(void);
void lwip_pkt_handle(void);
void lwip_periodic_handle(__IO uint32_t localtime);

#endif /* NETCONF_H */
