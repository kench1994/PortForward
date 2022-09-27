#include "Frontend.h"
#include <boost/shared_array.hpp>
#include "utils/io_service_pool.hpp"
Frontend::Frontend()
	: m_abWorking(false)
{

}

Frontend::~Frontend()
{
}

int Frontend::initial(std::shared_ptr<socket>&& spSocket, \
 const fntOnNetPacket& fnOnPacket, const fntOnConnStatus& fnOnConnStatus)
{
	m_spSocket = spSocket;
	m_spStrand = IO_EXCUTOR.pick_blader()->spStrand;
	m_fnOnPacket = fnOnPacket;
	m_fnOnConnStatus = fnOnConnStatus;
	return 0;
}

void Frontend::stop()
{

}

void Frontend::onRecv( \
	boost::shared_array<char>spszBuff, \
	unsigned int uRecvSize, \
	const boost::system::error_code& ec
) 
{
	if (m_fnOnPacket) {
		m_fnOnPacket(std::move(spszBuff), uRecvSize, ec.value(), ec.message().c_str());
	}

	//TODO: �����쳣����
	if (ec) {
		m_abWorking = false;
		return;
	}

	doRecv();
}
