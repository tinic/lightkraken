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
#ifndef _SACN_H_
#define _SACN_H_

namespace lightkraken {

class sACNPacket {

public:

	enum {
		VECTOR_ROOT_E131_DATA = 0x00000004,
		VECTOR_ROOT_E131_EXTENDED = 0x00000008,
		VECTOR_E131_DATA_PACKET = 0x00000002,
		VECTOR_DMP_SET_PROPERTY = 0x02,
		VECTOR_E131_EXTENDED_SYNCHRONIZATION = 0x00000001,
		VECTOR_E131_EXTENDED_DISCOVERY = 0x00000002,
		VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST = 0x00000001,
		E131_DISCOVERY_UNIVERSE = 64214,
		ACN_SDT_MULTICAST_PORT = 5568
	};

    enum PacketType {
    	PacketInvalid			= -1,
    	PacketData				=  0,
    	PacketSync				=  1,
    	PacketDiscovery			=  2
    };

    static bool dispatch(const ip_addr_t *from, const uint8_t *buf, size_t len, bool isBroadcast);
    static void sendDiscovery();
	static void maybeJoinNetworks();
	static void maybeLeaveNetworks();

protected:

    sACNPacket() { };
    virtual bool verify() const { return false; }
    uint8_t packet[1143];
    static uint16_t syncuniverse;

private:
	static PacketType maybeValid(const uint8_t *buf, size_t len);
    static bool verify(sACNPacket &Packet, const uint8_t *buf, size_t len);

};

}

#endif  // #ifndef _SACN_H_
