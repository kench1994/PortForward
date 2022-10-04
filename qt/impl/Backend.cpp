#include "Backend.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "utils/io_service_pool.hpp"


Backend::Backend()
{
}

Backend::~Backend()
{
	//while (m_Queue.size()){
	//	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	//}
}


int Backend::initial(const std::shared_ptr<NodeInfo>& spNodeInfo, \
 const fntOnNetPacket& fnOnPacket, const fntOnConnStatus& fnOnConnStatus)
{
	m_spNodeInfo = spNodeInfo;
	m_fnOnPacket = fnOnPacket;
	m_fnOnConnStatus = fnOnConnStatus;
	return 0;
}


void Backend::stop()
{
	if (m_spSocket->is_open())
		m_spSocket->close();
}

int Backend::connRemote()
{
	//���ظ�����
	if (_enConnStatus::connected == m_auStaus.load())
		return 0;

	//��������
	boost::asio::ip::tcp::resolver resolver(*m_spIO);
	boost::asio::ip::tcp::resolver::query query(\
		m_spNodeInfo->strHost.c_str(), m_spNodeInfo->strPort.c_str()
	);

	boost::system::error_code ec;
	auto resolver_result = resolver.resolve(query, ec);
	if (ec) {
		//TODO:֪ͨʧ��
		if(m_fnOnConnStatus)
			m_fnOnConnStatus(ec.value(), ec.message().c_str());
		return -1;
	}

	auto spSocket = std::make_shared<socket>(*m_spIO);
	boost::asio::async_connect(*spSocket, resolver_result,
		std::bind(&Backend::onConned, this, spSocket, std::placeholders::_1)
	);
	//�첽 conn �����ύ�ɹ�
	return 0;
}

void Backend::onConned(std::shared_ptr<socket>spSocket, const boost::system::error_code ec)
{
	if (m_fnOnConnStatus)
		m_fnOnConnStatus(ec.value(), ec.message().c_str());

	if (ec.value()) {
		m_auStaus.store(_enConnStatus::unconn);
		return;
	}

	m_auStaus.store(_enConnStatus::connected);
	m_spSocket = std::move(spSocket);

	doRecv();
}
