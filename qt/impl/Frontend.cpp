#include "Frontend.h"
#include <boost/shared_array.hpp>
Frontend::Frontend()
{

}

Frontend::~Frontend()
{
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
	if (m_spSocket->is_open())
		m_spSocket->close();
}
