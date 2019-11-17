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
#include "lwip/igmp.h"
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

    uint16_t syncuniverse() const { return (packet[109] << 8 ) | (packet[110] << 0 ); };
    uint16_t universe() const { return (packet[113] << 8 ) | (packet[114] << 0 ); };
    size_t datalen() const { return (packet[123] << 8 ) | (packet[124] << 0 ); };
    const uint8_t *data() const { return &packet[125]; }

private:
    
    virtual bool verify() const override {
        if (datalen() > 513 ||
            datalen() <   1 ) {
            return false;
        }
        if (packet[118] != 0xa1) {
            return false;
        }
        if ( ((packet[119] << 8 ) | (packet[120] << 0)) != 0x0000) {
            return false;
        }
        if ( ((packet[121] << 8 ) | (packet[122] << 0)) != 0x0001) {
            return false;
        }
        return true;
    }
};
    
class SyncPacket : public sACNPacket {
public:
    SyncPacket() { };

    uint16_t syncuniverse() const { return (packet[45] << 8 ) | (packet[46] << 0 ); };
    
private:
    virtual bool verify() const override {
        return true;
    }
};

class DiscoveryPacket : public sACNPacket {
public:
    DiscoveryPacket() { };
private:
    virtual bool verify() const override {
        return true;
    }
};

sACNPacket::PacketType sACNPacket::maybeValid(const uint8_t *buf, size_t len) {

    if (len > sizeof(packet)) {
        return PacketInvalid;
    }

    if (len < 48) {
        return PacketInvalid;
    }

    if (((buf[0] << 8) | (buf[1] << 0)) != 0x0010) {
        return PacketInvalid;
    }

    if (((buf[2] << 8) | (buf[3] << 0)) != 0x0000) {
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

void sACNPacket::sendDiscovery() {
    struct sACNDiscovery {
        uint16_t preambleSize;
        uint16_t postPreambleSize;
        uint8_t packetIdentifier[12];
        uint16_t flagsAndLengthRoot;
        uint32_t vectorRoot;
        uint8_t cid[16];
        uint16_t flagsAndLengthFraming;
        uint32_t vectorFraming;
        uint8_t sourceName[64];
        uint32_t reserved;
        uint16_t flagsAndLengthDiscovery;
        uint32_t vectorDiscovery;
        uint8_t page;
        uint8_t last;
        uint16_t universes[Model::maxUniverses];
    }  __attribute__((packed)) discovery;

    size_t universeCount = 0;
    std::array<uint16_t, Model::maxUniverses> universes;
    Control::instance().collectAllActiveE131Universes(universes, universeCount);
    std::sort(universes.begin(), universes.begin()+universeCount);  
    size_t replySize = offsetof(sACNDiscovery, universes)+universeCount*sizeof(uint16_t);
    
    auto hton16 = [] (uint16_t v) {
        return uint16_t((v>>8)|(v<< 8));
    };
    auto hton32 = [] (uint32_t v) {
        return uint32_t(((v>>24)&0x000000FF)|((v>>8)&0x0000FF00)|((v<<24)&0xFF000000)|((v<<8)&0x00FF0000));
    };
    memset(&discovery, 0, sizeof(discovery));
    discovery.preambleSize = hton16(0x0010);
    discovery.postPreambleSize = hton16(0x0000);
    memcpy(&discovery.packetIdentifier[0], "ASC-E1.17\0\0\0", 12);
    discovery.flagsAndLengthRoot = hton16(0x7000 + replySize - offsetof(sACNDiscovery, flagsAndLengthRoot));
    discovery.vectorRoot = hton32(VECTOR_ROOT_E131_EXTENDED);
    discovery.flagsAndLengthFraming = hton16(0x7000 + replySize - offsetof(sACNDiscovery, flagsAndLengthFraming));
    for (size_t c=0; c<16; c++) {
        discovery.cid[c] = NetConf::instance().netInterface()->hwaddr[c % 6];
    }
    discovery.vectorFraming = hton32(VECTOR_E131_EXTENDED_DISCOVERY);
    strcpy(reinterpret_cast<char *>(&discovery.sourceName[0]),NetConf::instance().netInterface()->hostname);
    discovery.flagsAndLengthDiscovery = hton16(0x7000 + replySize - offsetof(sACNDiscovery, flagsAndLengthDiscovery));
    discovery.vectorDiscovery = hton32(VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST);
    discovery.page = 0;
    discovery.last = 0;
    for (size_t c = 0; c < universeCount; c++) {
        discovery.universes[c] = hton16(universes[c]);
    }

    struct ip4_addr broadcastAddr;
    broadcastAddr.addr =  (NetConf::instance().netInterface()->ip_addr.addr &
                        NetConf::instance().netInterface()->netmask.addr) | 
                        ~NetConf::instance().netInterface()->netmask.addr;    
    NetConf::instance().sendsACNUdpPacket(&broadcastAddr, ACN_SDT_MULTICAST_PORT, (const uint8_t *)&discovery, replySize);
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
                        lightkraken::Control::instance().setUniverseOutputData(dataPacket.universe(), dataPacket.data() + 1, dataPacket.datalen() - 1);
                        syncuniverse = dataPacket.syncuniverse();
                        if (dataPacket.syncuniverse() == 0) {
                            Control::instance().sync();
                            Control::instance().setEnableSyncMode(false);
                        }
                        return true;
                    }
                } break;
        case	PacketSync: {
                    if (!Model::instance().broadcastEnabled() && isBroadcast) {
                        return false;
                    }
                    SyncPacket syncPacket;
                    if (sACNPacket::verify(syncPacket, buf, len)) {
                        if (syncPacket.syncuniverse() == syncuniverse) {
                            Control::instance().sync();
                            Control::instance().setEnableSyncMode(false);
                        }
                        return true;
                    }
                } break;
        case	PacketDiscovery: {
                    DiscoveryPacket discoveryPacket;
                    if (sACNPacket::verify(discoveryPacket, buf, len)) {
                        return true;
                    }
                } break;
        default:
        case	PacketInvalid: {
                    return false;
                } break;
    }
    return false;
}

void sACNPacket::leaveNetworks() {
    size_t universeCount = 0;
    std::array<uint16_t, Model::maxUniverses> universes;
    Control::instance().collectAllActiveE131Universes(universes, universeCount);
    for (size_t c = 0; c < universeCount; c++) {
        ip4_addr multicast_addr;
        IP4_ADDR(&multicast_addr, 239, 255, (universes[c] >> 8) & 0xFF, (universes[c] >> 0) & 0xFF);
        if (igmp_lookfor_group(NetConf::instance().netInterface(), &multicast_addr) == 0) {
            igmp_leavegroup_netif(NetConf::instance().netInterface(), &multicast_addr);
        }
    }
}

void sACNPacket::joinNetworks() {
    size_t universeCount = 0;
    std::array<uint16_t, Model::maxUniverses> universes;
    Control::instance().collectAllActiveE131Universes(universes, universeCount);
    for (size_t c = 0; c < universeCount; c++) {
        ip4_addr multicast_addr;
        IP4_ADDR(&multicast_addr, 239, 255, (universes[c] >> 8) & 0xFF, (universes[c] >> 0) & 0xFF);
        if (igmp_lookfor_group(NetConf::instance().netInterface(), &multicast_addr) == 0) {
            igmp_joingroup_netif(NetConf::instance().netInterface(), &multicast_addr);
        }
    }
}

uint16_t sACNPacket::syncuniverse = 0;

}

