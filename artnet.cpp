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

namespace lightguy {

	class Packet {
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

		Packet() { };

		virtual bool verify() const { return false; }

		uint8_t packet[512+18];

		Opcode opcode() const;
		int version() const;

	private:
		static Opcode maybeValid(const uint8_t *buf, size_t len);
		static bool verify(Packet &Packet, const uint8_t *buf, size_t len);
	};

	class OutputPacket : public Packet {
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

	class OutputNzsPacket : public Packet {
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

	int Packet::version() const {
		return static_cast<int>((packet[10] << 8) | (packet[11]));
	}

	Packet::Opcode Packet::opcode() const {
		return static_cast<Opcode>((packet[8]) | (packet[9] << 8));
	}

	Packet::Opcode Packet::maybeValid(const uint8_t *buf, size_t len) {

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

	bool Packet::verify(Packet &packet, const uint8_t *buf, size_t len) {
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

	bool Packet::dispatch(const uint8_t *buf, size_t len) {
		Packet::Opcode opcode = Packet::maybeValid(buf, len);
		if (opcode != OpInvalid) {
			switch(opcode) {
				case	OpSync: {
						} break;
				case	OpNzs: {
							OutputNzsPacket outputPacket;
							if (Packet::verify(outputPacket, buf, len)) {
								lightguy::Model::instance().setUniverseOutputData(outputPacket.universe(), outputPacket.data(), outputPacket.len());
							}
							return true;
						} break;
				case	OpOutput: {
							OutputPacket outputPacket;
							if (Packet::verify(outputPacket, buf, len)) {
								lightguy::Model::instance().setUniverseOutputData(outputPacket.universe(), outputPacket.data(), outputPacket.len());
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

void lightguy_artnet_packet_dispatch(uint8_t *data, size_t len) {
	lightguy::Packet::dispatch(data, len);
}

