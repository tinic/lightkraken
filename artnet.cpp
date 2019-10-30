/*
* artnet.cpp
*
*  Created on: Sep 5, 2019
*      Author: turo
*/
#include <stdint.h>
#include <string.h>
#include <algorithm>

extern "C" {
#include "lwip/udp.h"
}; //extern "C" {

#include "./artnet.h"
#include "./model.h"
#include "./control.h"
#include "./systick.h"
#include "./netconf.h"

namespace lightkraken {

static const int32_t build_number = 
#include "./build_number.h"
;

static ArtSyncWatchDog syncWatchDog;

void ArtSyncWatchDog::feed() {
	fedtime = Systick::instance().systemTime();
}

bool ArtSyncWatchDog::starved() {
	uint32_t now = Systick::instance().systemTime();
	if (fedtime == 0 || ((now - fedtime) > ArtSyncTimeout )) {
		fedtime = 0;
		return true;
	}
	return false;
}

class OutputPacket : public ArtNetPacket {
public:
    OutputPacket() { };

    size_t len() const;
    uint8_t sequence() const;
    uint8_t physical() const;
    uint16_t universe() const;

    const uint8_t *data() const { return &packet[18]; }

private:

    virtual bool verify() const override;
};

class OutputNzsPacket : public ArtNetPacket {
public:
    OutputNzsPacket() { };

    size_t len() const;
    uint8_t sequence() const;
    uint8_t startCode() const;
    uint16_t universe() const;

    const uint8_t *data() const { return &packet[18]; }

private:

    virtual bool verify() const override;
};

int ArtNetPacket::version() const {
    return static_cast<int>((packet[10] << 8) | (packet[11]));
}

ArtNetPacket::Opcode ArtNetPacket::opcode() const {
    return static_cast<Opcode>((packet[8]) | (packet[9] << 8));
}

ArtNetPacket::Opcode ArtNetPacket::maybeValid(const uint8_t *buf, size_t len) {

    bool bufValid = buf ? true : false;

    bool sizeValid = len <= sizeof(packet);

    bool validSignature =
        (buf[0] == 'A') &&
        (buf[1] == 'r') &&
        (buf[2] == 't') &&
        (buf[3] == '-') &&
        (buf[4] == 'N') &&
        (buf[5] == 'e') &&
        (buf[6] == 't') &&
        (buf[7] == 0);

    bool opcodeValid = false;

    Opcode opcode = static_cast<Opcode>((buf[8]) | (buf[9] << 8));
    switch (opcode) {
        case	OpPoll:
        case	OpPollReply:
        case	OpDiagData:
        case	OpCommand:
        case	OpOutput:
        case	OpNzs:
        case	OpSync:
        case	OpAddress:
        case	OpInput:
        case	OpTodRequest:
        case	OpTodData:
        case	OpTodControl:
        case	OpRdm:
        case	OpRdmSub:
        case	OpVideoSetup:
        case	OpVideoPalette:
        case	OpVideoData:
        case	OpMacMaster:
        case	OpMacSlave:
        case	OpFirmwareMaster:
        case	OpFirmwareReply:
        case	OpFileTnMaster:
        case	OpFileFnMaster:
        case	OpFileFnReply:
        case	OpIpProg:
        case	OpIpProgReply:
        case	OpMedia:
        case	OpMediaPatch:
        case	OpMediaControl:
        case	OpMediaContrlReply:
        case	OpTimeCode:
        case	OpTimeSync:
        case	OpTrigger:
        case	OpDirectory:
        case	OpDirectoryReply:
                opcodeValid = true;
                break;
        default:
                opcodeValid = false;
                break;
    }

    bool versionValid = static_cast<int>((buf[10] << 8) | (buf[11])) >= currentVersion;

    return 	(bufValid &&
            sizeValid &&
            validSignature &&
            opcodeValid &&
            versionValid) ? opcode : OpInvalid;
}

bool ArtNetPacket::verify(ArtNetPacket &packet, const uint8_t *buf, size_t len) {
    Opcode opcode = maybeValid(buf, len);
    if (opcode == OpInvalid) {
        return false;
    }
    memset(packet.packet, 0, sizeof(packet.packet));
    memcpy(packet.packet, buf, std::min(len, sizeof(packet.packet)));
    switch (opcode) {
      	case	OpPoll:
        case 	OpSync:
        case	OpNzs:
        case	OpOutput: {
                    return packet.verify();
                } break;
        default: {
                    return false;
                } break;
    }
    return false;
}

static constexpr uint32_t syncTimeout = 4;

void ArtNetPacket::sendArtPollReply(const ip_addr_t *from, uint16_t universe) {

	struct ArtPollReply {
		uint16_t opCode;
		uint8_t  ipAddress[4];
		uint16_t portNumber;
		uint16_t versionInfo;
		uint8_t  netSwitch;
		uint8_t  subSwitch;
		uint16_t oem;
		uint8_t  uebaVersion;
		uint8_t  status1;
		uint16_t estaManufactor;
		uint8_t  shortName[18];
		uint8_t  longName[64];
		uint8_t  nodeReport[64];
		uint16_t numPorts;
		uint8_t  portTypes[4];
		uint8_t  goodInput[4];
		uint8_t  goodOutput[4];
		uint8_t  swIn[4];
		uint8_t  swOut[4];
		uint8_t  swVideo;
		uint8_t  swMacro;
		uint8_t  swRemote;
		uint8_t  spare1;
		uint8_t  spare2;
		uint8_t  spare3;
		uint8_t  style;
		uint8_t  macAddress[6];
		uint8_t  bindIp[4];
		uint8_t  bindIndex;
		uint8_t  status;
		uint8_t  filler[26];
	}  __attribute__((packed)) reply;

	memset(&reply, 0, sizeof(reply));
	reply.opCode = OpPollReply;
	reply.ipAddress[0] = ip4_addr1(&NetConf::instance().netInterface()->ip_addr);
	reply.ipAddress[1] = ip4_addr2(&NetConf::instance().netInterface()->ip_addr);
	reply.ipAddress[2] = ip4_addr3(&NetConf::instance().netInterface()->ip_addr);
	reply.ipAddress[3] = ip4_addr4(&NetConf::instance().netInterface()->ip_addr);
	reply.portNumber = 1936;
	reply.versionInfo = build_number;
	reply.netSwitch = (universe >> 8) & 0xFF;
	reply.subSwitch = (universe >> 0) & 0xFF;
	reply.oem = 0x1ed5;
	reply.uebaVersion = 0;
	reply.status1 = 0;
	reply.estaManufactor = 0;

	NetConf::instance().sendUdpPacket(from, 1936, (const uint8_t *)&reply, sizeof(reply));
}

bool ArtNetPacket::dispatch(const ip_addr_t *from, const uint8_t *buf, size_t len) {
    ArtNetPacket::Opcode opcode = ArtNetPacket::maybeValid(buf, len);
    if (opcode != OpInvalid) {
        switch(opcode) {
        	case	OpPoll: {
        				Control::instance().interateAllActiveUniverses([from](uint16_t universe){ 
        					sendArtPollReply(from, universe); 
        				});
        			} break;
            case	OpSync: {
            			Control::instance().setEnableSyncMode(true);
						Control::instance().sync();
            			syncWatchDog.feed();
                    } break;
            case	OpNzs: {
                        OutputNzsPacket outputPacket;
                        if (ArtNetPacket::verify(outputPacket, buf, len)) {
                            lightkraken::Control::instance().setUniverseOutputData(outputPacket.universe(), outputPacket.data(), outputPacket.len());
                        }
            			if(syncWatchDog.starved()) {
	            			Control::instance().sync();
 	            			Control::instance().setEnableSyncMode(false);
	           			}
                        return true;
                    } break;
            case	OpOutput: {
                        OutputPacket outputPacket;
                        if (ArtNetPacket::verify(outputPacket, buf, len)) {
                            lightkraken::Control::instance().setUniverseOutputData(outputPacket.universe(), outputPacket.data(), outputPacket.len());
                        }
            			if(syncWatchDog.starved()) {
	            			Control::instance().sync();
 	            			Control::instance().setEnableSyncMode(false);
	           			}
                        return true;
                    } break;
            default: {
                        return false;
                    } break;
        }
    }
    return false;
}

size_t OutputPacket::len() const {
    return (packet[16] << 8) | packet[17];
}

uint16_t OutputPacket::universe() const {
    return (packet[14]) | (packet[15]<<8);
}

uint8_t OutputPacket::sequence() const {
    return packet[12];
}

uint8_t OutputPacket::physical() const {
    return packet[13];
}

bool OutputPacket::verify() const {
    if (len() < 2) {
        return false;
    }
    if (len() > 512) {
        return false;
    }
    if ((len() & 1) == 1) {
        return false;
    }
    if (universe() >= 32768) {
        return false;
    }
    return true;
}

size_t OutputNzsPacket::len() const {
    return (packet[16] << 8) | packet[17];
}

uint16_t OutputNzsPacket::universe() const {
    return (packet[14]) | (packet[15]<<8);
}

uint8_t OutputNzsPacket::sequence() const {
    return packet[12];
}

uint8_t OutputNzsPacket::startCode() const {
    return packet[13];
}

bool OutputNzsPacket::verify() const {
    if (len() < 2) {
        return false;
    }
    if (len() > 512) {
        return false;
    }
    if ((len() & 1) == 1) {
        return false;
    }
    if (universe() >= 32768) {
        return false;
    }
    if (startCode() != 0) {
        return false;
    }
    return true;
}
};

