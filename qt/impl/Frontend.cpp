#include "Frontend.h"
#include <boost/shared_array.hpp>
#include <QDebug>
Frontend::Frontend()
{

}

Frontend::~Frontend()
{
	qDebug() << "Frontend deconstruct";

	if (m_fnOnConnStatus) {
		m_fnOnConnStatus(-1, "deconstruct");
		m_fnOnConnStatus = NULL;
	}
	m_fnOnPacket = NULL;
}

int Frontend::initial(std::shared_ptr<socket>&& spSocket, \
 const fntOnNetPacket& fnOnPacket, const fntOnConnStatus& fnOnConnStatus)
{
	m_spSocket = std::move(spSocket);
	m_fnOnPacket = fnOnPacket;
	m_fnOnConnStatus = fnOnConnStatus;
	m_auStaus.store(_enConnStatus::connected);
	return 0;
}

void Frontend::stop()
{
	if (!(m_uShutdownState & 0x01)) {
		m_spSocket->shutdown(boost::asio::socket_base::shutdown_receive);
		m_uShutdownState |= 0x01;
	}
	if (!(m_uShutdownState & 0x01)) {
		m_spSocket->shutdown(boost::asio::socket_base::shutdown_send);
		m_uShutdownState |= 0x10;
	}
}
