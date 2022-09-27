#include "Backend.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "utils/io_service_pool.hpp"


Backend::Backend()
	: m_auStaus(_enConnStatus::unconn)
{
}

Backend::~Backend()
{
	while (m_Queue.size()){
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}


int Backend::initial(const std::shared_ptr<NodeInfo>& spNodeInfo, const fntOnNetPacket& fnOnPacket)
{
	m_spNodeInfo = spNodeInfo;
	m_fnOnPacket = fnOnPacket;
	return 0;
}


void Backend::stop()
{

}

int Backend::connRemote(const fntOnConnStatus &fnOnStatus)
{
	//不重复连接
	if (_enConnStatus::connected == m_auStaus.load())
		return 0;
	//顺序执行器
	auto *pBlader = IO_EXCUTOR.pick_blader();
	//TODO:???
	if(!m_spStrand)
		m_spStrand = pBlader->spStrand;

	//域名解析
	boost::asio::ip::tcp::resolver resolver(*pBlader->spIO);
	boost::asio::ip::tcp::resolver::query query(\
		m_spNodeInfo->strHost.c_str(), m_spNodeInfo->strPort.c_str()
	);

	boost::system::error_code ec;
	auto resolver_result = resolver.resolve(query, ec);
	if (ec) {
		//TODO:通知失败
		fnOnStatus(ec.value(), ec.message().c_str());
		return -1;
	}
	m_fnOnConnStatus = fnOnStatus;
	auto spSocket = std::make_shared<socket>(resolver.get_executor());
	boost::asio::async_connect(*spSocket, resolver_result,
		std::bind(&Backend::onConned, this, spSocket, std::placeholders::_1)
	);
	//异步 conn 任务提交成功
	return 0;
}
//
//int Backend::addToSendChains(const std::shared_ptr<PACKET>& spPacket)
//{
//	if (_enConnStatus::connected != m_auStaus.load())
//		return -1;
//
//	//TODO:stop
//	//auto spThis = shared_from_this();
//	m_spStrand->post([this, spPacket]()
//	{
//		m_Queue.push(std::move(spPacket));
//
//		//触发发送
//		if (1 == m_Queue.size())
//			triggerSend(spPacket);
//	});
//
//	return 0;
//}

//int Backend::toldShutdown()
//{
//	return 0;
//}
//
//_enConnStatus Backend::getStatus()
//{
//	auto s = m_auStaus.load();
//	return s;
//}

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
//
//void Backend::triggerSend(const std::shared_ptr<PACKET>& spPacket)
//{
//	m_spSocket->async_send(boost::asio::buffer(spPacket->spszBuff.get(), spPacket->uRealLen),
//		std::bind(&Backend::onSend, this, std::placeholders::_1)
//	);
//}
//
//void Backend::onSend(const boost::system::error_code& ec)
//{
//	if (ec) {
//		//通知失败,断开连接,清空队列
//		return;
//	}
//
//	m_spStrand->post([this](){
//		if (!m_Queue.empty())
//			m_Queue.pop();
//
//		//Read_Lock rStopStatusLock(m_rwmutStoped);
//		//if (m_bStoped)
//		//	return;
//
//		if (!m_Queue.empty()){
//			auto spNextSend = m_Queue.front();
//			triggerSend(spNextSend);
//		}
//	});
//}


//void Backend::doRecv()
//{
//	constexpr unsigned int uBufferSize = 4096;
//	boost::shared_array<char>spszBuffer(new char[uBufferSize]);
//	memset(spszBuffer.get(), 0, uBufferSize);
//
//	//m_abWorking = true;
//
//	m_spSocket->async_read_some(boost::asio::buffer(spszBuffer.get(), uBufferSize),
//		boost::asio::bind_executor(*m_spStrand,
//		std::bind(&Backend::onRecv, this,
//			std::placeholders::_1,
//			std::placeholders::_2,
//			spszBuffer
//		))
//	);
//}

//void Backend::onRecv(\
//	const boost::system::error_code& ec,
//	size_t  uRecvSize,
//	boost::shared_array<char>spszBuff
//)
//{
//	//TODO
//	//if (m_fnOnDataIncomm) {
//	//	m_fnOnDataIncomm(std::move(spszBuff), uRecvSize, ec.value(), ec.message().c_str());
//	//}
//
//	//TODO: 连接异常处理
//	if (ec) {
//		//m_abWorking = false;
//		return;
//	}
//
//	doRecv();
//}
