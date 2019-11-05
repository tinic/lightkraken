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
#ifndef _ARTNET_H_
#define _ARTNET_H_

namespace lightkraken {

class ArtSyncWatchDog {
public:
	bool starved();
	void feed();
private:
	constexpr static uint32_t ArtSyncTimeout = 4000;
	uint32_t fedtime = 0;
};

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

    static bool dispatch(const ip_addr_t *from, const uint8_t *buf, size_t len);

protected:

    ArtNetPacket() { };

    virtual bool verify() const { return false; }
    uint8_t packet[512+18];
    Opcode opcode() const;
    int version() const;

private:
	static void sendArtPollReply(const ip_addr_t *from, uint16_t universe);
    static Opcode maybeValid(const uint8_t *buf, size_t len);
    static bool verify(ArtNetPacket &Packet, const uint8_t *buf, size_t len);
};

}  // namespace lightkraken {

#endif /* _ARTNET_H_ */
