/*
 * artnet.h
 *
 *  Created on: Sep 18, 2019
 *      Author: Tinic Uro
 */

#ifndef LIGHTGUY_ARTNET_H_
#define LIGHTGUY_ARTNET_H_

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

void lightguy_artnet_packet_dispatch(uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus

#endif /* LIGHTGUY_ARTNET_H_ */
