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

extern "C" {
#include "lwip/udp.h"
}; //extern "C" {

#include "./sacn.h"
#include "./artnet.h"
#include "./model.h"
#include "./control.h"
#include "./systick.h"
#include "./netconf.h"
#include "./perf.h"

namespace lightkraken {

class DataPacket : public sACNPacket {
public:
    DataPacket() { };

    uint16_t universe() const { return (packet[113] << 8 ) | (packet[114] << 0 ); };
    size_t len() const { return (packet[123] << 8 ) | (packet[124] << 0 ); };
    const uint8_t *data() const { return &packet[126]; }

private:
    virtual bool verify() const override;
};

class SyncPacket : public sACNPacket {
public:
    SyncPacket() { };
private:
    virtual bool verify() const override;
};

class DiscoveryPacket : public sACNPacket {
public:
    DiscoveryPacket() { };
private:
    virtual bool verify() const override;
};

sACNPacket::PacketType sACNPacket::maybeValid(const uint8_t *buf, size_t len) {

	if (len < 48) {
		return PacketInvalid;
	}

	if (((buf[0]) | (buf[1] << 8)) != 0x010) {
		return PacketInvalid;
	}

	if (((buf[2] << 8) | (buf[3] << 0)) != 0x010) {
		return PacketInvalid;
	}

	if (memcmp(&buf[4], "ASC-E1.17\0\0\0", 12) != 0){
		return PacketInvalid;
	}

	uint32_t protocolType = (buf[18] << 24) | (buf[19] << 16) | (buf[20] <<  8) | (buf[21] <<  0);
	switch (protocolType) {
		case VECTOR_ROOT_E131_DATA: {
			uint32_t frameType = (buf[40] << 24) | (buf[41] << 16) | (buf[42] <<  8) | (buf[43] <<  0);
			switch (frameType) {
				case VECTOR_E131_DATA_PACKET: {
					if (len < 125) {
						return PacketInvalid;
					}
					uint32_t dmpType = buf[117];
					switch (dmpType) {
						case VECTOR_DMP_SET_PROPERTY: {
							return PacketData;
						} break;
						default: {
							return PacketInvalid;
						} break;
					}
				} break;
				default: {
					return PacketInvalid;
				} break;
			}
		} break;
		case VECTOR_ROOT_E131_EXTENDED: {
			uint32_t frameType = (buf[40] << 24) | (buf[41] << 16) | (buf[42] <<  8) | (buf[43] <<  0);
			switch (frameType) {
				case VECTOR_E131_EXTENDED_SYNCHRONIZATION: {
					return PacketSync;
				} break;
				case VECTOR_E131_EXTENDED_DISCOVERY: {
					if (len < 119) {
						return PacketInvalid;
					}
					uint32_t discoveryType = (buf[114] << 24) | (buf[115] << 16) | (buf[116] <<  8) | (buf[117] <<  0);
					switch (discoveryType) {
						case VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST: {
							return PacketDiscovery;
						} break;
						default: {
							return PacketInvalid;
						} break;
					}
				} break;
				default: {
					return PacketInvalid;
				} break;
			}
		} break;
	}

	return PacketInvalid;
}

bool sACNPacket::verify(sACNPacket &packet, const uint8_t *buf, size_t len) {
    PacketType type = sACNPacket::maybeValid(buf, len);
    if (type == PacketInvalid) {
    	return false;
    }
    memcpy(packet.packet, buf, std::min(len, sizeof(packet.packet)));
    switch (type) {
		case	PacketData:
		case	PacketSync:
		case	PacketDiscovery: {
                    return packet.verify();
                } break;
        default:
		case	PacketInvalid: {
			    	return false;
				} break;
    }
    return false;
}

bool sACNPacket::dispatch(const ip_addr_t *from, const uint8_t *buf, size_t len, bool isBroadcast) {
	(void)from;
	
	PerfMeasure perf(PerfMeasure::SLOT_SACN_DISPATCH);
    PacketType type = sACNPacket::maybeValid(buf, len);
    if (type == PacketInvalid) {
    	return false;
    }
	switch(type) {
		case	PacketData: {
					if (!Model::instance().broadcastEnabled() && isBroadcast) {
						return false;
					}
					DataPacket dataPacket;
					if (sACNPacket::verify(dataPacket, buf, len)) {
						lightkraken::Control::instance().setUniverseOutputData(dataPacket.universe(), dataPacket.data(), dataPacket.len());
					}
				} break;
		case	PacketSync: {
					SyncPacket syncPacket;
					if (sACNPacket::verify(syncPacket, buf, len)) {
					}
				} break;
		case	PacketDiscovery: {
					DiscoveryPacket discoveryPacket;
					if (sACNPacket::verify(discoveryPacket, buf, len)) {
					}
				} break;
        default:
		case	PacketInvalid: {
			    	return false;
				} break;
	}
	return false;
}

bool DataPacket::verify() const {
	return false;
}

bool SyncPacket::verify() const {
	return false;
}

bool DiscoveryPacket::verify() const {
	return false;
}

}

