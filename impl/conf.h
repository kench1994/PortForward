#pragma once
#include <cstdint>
#include <boost/shared_array.hpp>
enum _enConnStatus : uint8_t {
	unconn = 0x00,
	connected
};

typedef struct tagPacket {
	tagPacket()
	 : uRealLen(0) {}
	tagPacket(boost::shared_array<char>&&buf, unsigned int uLen)
		: spszBuff(std::move(buf)), uRealLen(uLen) {}
	boost::shared_array<char> spszBuff;
	unsigned int uRealLen;
}PACKET, *P_PACKET;


#define CHECK_PROMT_WSP(sp, wsp) (nullptr != (sp = wsp.lock()))
#define CHECK_PROMT_WSP_FAILED(sp, wsp) (nullptr == (sp = wsp.lock()))