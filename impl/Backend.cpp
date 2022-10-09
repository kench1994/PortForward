#include "Backend.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "utils/io_service_pool.hpp"
#include "utils/cost_timer.hpp"
#include <QDebug>

Backend::Backend()
{
}

Backend::~Backend()
{
	qDebug() << "Backend deconstruct";

	if (m_fnOnConnStatus) {
		m_fnOnConnStatus(-1, "deconstruct");
		m_fnOnConnStatus = NULL;
	}
	m_fnOnPacket = NULL;
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
	std::unique_lock<std::mutex> lck(m_mtxShutdownState);
	if (!(m_uShutdownState & 0x01)) {
		m_spSocket ? m_spSocket->shutdown(boost::asio::socket_base::shutdown_receive) : 0;
		m_uShutdownState |= 0x01;
	}
	if (!(m_uShutdownState & 0x10)) {
		m_spSocket ? m_spSocket->shutdown(boost::asio::socket_base::shutdown_send) : 0;
		m_uShutdownState |= 0x10;
	}
}

int Backend::connRemote()
{
	//不重复连接
	if (_enConnStatus::connected == m_auStaus.load())
		return 0;

	//域名解析
	boost::asio::ip::tcp::resolver resolver(*m_spIO);
	boost::asio::ip::tcp::resolver::query query(\
		m_spNodeInfo->strHost.c_str(), m_spNodeInfo->strPort.c_str()
	);

	boost::system::error_code ec;
	auto resolver_result = resolver.resolve(query, ec);
	if (ec) {
		//TODO:通知失败
		if (m_fnOnConnStatus) {
			m_fnOnConnStatus(ec.value(), ec.message().c_str());
			m_fnOnConnStatus = NULL;
		}
		return -1;
	}

	auto spSocket = std::make_shared<socket>(*m_spIO);
	boost::asio::async_connect(*spSocket, resolver_result,
		boost::asio::bind_executor(*m_spStrand, 
			std::bind(&Backend::onConned, this, spSocket, std::placeholders::_1)
		)
	);
	//异步 conn 任务提交成功
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

	if (m_Queue.size()) {
		triggerSend(nullptr);
	}
	
	doRecv();
}
