/*
* artnet.cpp
*
*  Created on: Sep 5, 2019
*      Author: turo
*/
#include <stdint.h>
#include <memory.h>
#include <algorithm>

#include "./artnet.h"
#include "./model.h"
#include "./control.h"

namespace lightguy {

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

bool ArtNetPacket::dispatch(const uint8_t *buf, size_t len) {
    ArtNetPacket::Opcode opcode = ArtNetPacket::maybeValid(buf, len);
    if (opcode != OpInvalid) {
        switch(opcode) {
            case	OpSync: {
                    } break;
            case	OpNzs: {
                        OutputNzsPacket outputPacket;
                        if (ArtNetPacket::verify(outputPacket, buf, len)) {
                            lightguy::Control::instance().setUniverseOutputData(outputPacket.universe(), outputPacket.data(), outputPacket.len());
                        }
                        return true;
                    } break;
            case	OpOutput: {
                        OutputPacket outputPacket;
                        if (ArtNetPacket::verify(outputPacket, buf, len)) {
                            lightguy::Control::instance().setUniverseOutputData(outputPacket.universe(), outputPacket.data(), outputPacket.len());
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

