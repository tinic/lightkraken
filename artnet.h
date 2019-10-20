/*
* artnet.h
*
*  Created on: Sep 18, 2019
*      Author: Tinic Uro
*/

#ifndef LIGHTGUY_ARTNET_H_
#define LIGHTGUY_ARTNET_H_

namespace lightguy {

class ArtNetPacket {
public:

    static constexpr int32_t currentVersion = 14;

    enum Opcode {
        OpInvalid			= -1,
        OpPoll 				= 0x2000,
        OpPollReply 		= 0x2100,
        OpDiagData 			= 0x2300,
        OpCommand 			= 0x2400,
        OpOutput 			= 0x5000,
        OpNzs 				= 0x5100,
        OpSync 				= 0x5200,
        OpAddress 			= 0x6000,
        OpInput 			= 0x7000,
        OpTodRequest 		= 0x8000,
        OpTodData 			= 0x8100,
        OpTodControl 		= 0x8200,
        OpRdm 				= 0x8300,
        OpRdmSub 			= 0x8400,
        OpVideoSetup 		= 0xa010,
        OpVideoPalette 		= 0xa020,
        OpVideoData 		= 0xa040,
        OpMacMaster 		= 0xf000,
        OpMacSlave 			= 0xf100,
        OpFirmwareMaster	= 0xf200,
        OpFirmwareReply 	= 0xf300,
        OpFileTnMaster 		= 0xf400,
        OpFileFnMaster 		= 0xf500,
        OpFileFnReply 		= 0xf600,
        OpIpProg 			= 0xf800,
        OpIpProgReply 		= 0xf900,
        OpMedia 			= 0x9000,
        OpMediaPatch 		= 0x9100,
        OpMediaControl 		= 0x9200,
        OpMediaContrlReply	= 0x9300,
        OpTimeCode 			= 0x9700,
        OpTimeSync 			= 0x9800,
        OpTrigger 			= 0x9900,
        OpDirectory 		= 0x9a00,
        OpDirectoryReply 	= 0x9b00
    };

    static bool dispatch(const uint8_t *buf, size_t len);

protected:

    ArtNetPacket() { };

    virtual bool verify() const { return false; }
    uint8_t packet[512+18];
    Opcode opcode() const;
    int version() const;

private:
    static Opcode maybeValid(const uint8_t *buf, size_t len);
    static bool verify(ArtNetPacket &Packet, const uint8_t *buf, size_t len);
};

}  // namespace lightguy {

#endif /* LIGHTGUY_ARTNET_H_ */
