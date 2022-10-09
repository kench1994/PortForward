#include "BaseConn.h"
#include "utils/io_service_pool.hpp"
#include <QDebug>

BaseConn::BaseConn()
	: m_spSocket(nullptr)
	, m_uShutdownState(0)
	, m_auStaus(_enConnStatus::unconn)
	, m_fnOnConnStatus(NULL)
	, m_fnOnPacket(NULL)
{
	auto pBlader = IO_EXCUTOR.pick_blader();
	m_spIO = pBlader->spIO;
	m_spStrand = pBlader->spStrand;
}


BaseConn::~BaseConn()
{
	//如果还未关闭发送通道等待发送完毕
	std::unique_lock<std::mutex> lck(m_mtxShutdownState);
	while (!(m_uShutdownState & 0x10) && m_Queue.size()) 
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	//等待接收通道关闭
	while (!(m_uShutdownState & 0x01))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	lck.unlock();


	if (m_fnOnConnStatus) {
		m_fnOnConnStatus(-1, "deconstruct");
		m_fnOnConnStatus = NULL;
	}
	m_fnOnPacket = NULL;

	qDebug() << m_strRole.data() << " (parent) deconstruct";
}

void BaseConn::setRole(const char* pszRole)
{ 
	if(!pszRole)
		return;
	m_strRole = pszRole; 
};

void BaseConn::shutdown(const unsigned int u)
{
	std::unique_lock<std::mutex> lck(m_mtxShutdownState);
	switch (u)
	{
	case 0x01:
		if (0x01 != (m_uShutdownState & 0x01) && m_spSocket)
			m_spSocket->shutdown_receive;
			break;
	case 0x10:
		if (0x10 != (m_uShutdownState & 0x10) && m_spSocket)
			m_spSocket->shutdown_send;
		break;
	default:
		break;
	}
	m_uShutdownState |= u;
}

int BaseConn::addToSendChains(const std::shared_ptr<PACKET>& spPacket)
{
	//目前连接上的状态是由外部管理的
	//frontend 直接等待 socket 转移所有者
	//backend 连接到 下游服务器后设置
	
	//适应 strand 队列避免锁竞争
	m_spStrand->post([this, spPacket](){
		//已关闭发送
		if (0x10 == (m_uShutdownState & 0x10)) {
			std::queue<std::shared_ptr<PACKET>> empty;
			m_Queue.swap(empty);
			return;
		}
		m_Queue.push(spPacket);
		
		//未连接上后端不触发实际发送
		if (_enConnStatus::connected != m_auStaus.load())
			return;

		//触发发送
		if (1 == m_Queue.size())
			triggerSend(spPacket);
	});

	return 0;
}


void BaseConn::triggerSend(const std::shared_ptr<PACKET>& spPacket)
{
	//传递进来假包,用于触发发送
	if (!spPacket) {
		auto spNextSend = m_Queue.front();
		triggerSend(spNextSend);
		return;
	}
	m_spSocket->async_send(boost::asio::buffer(spPacket->spszBuff.get(), spPacket->uRealLen),
		std::bind(&BaseConn::onSend, this, std::placeholders::_1, spPacket, shared_from_this())
	);
}

void BaseConn::onSend(const boost::system::error_code& ec, const std::shared_ptr<PACKET>& spPacket, std::weak_ptr<BaseConn> wspThis)
{
	std::shared_ptr<BaseConn> spConn{nullptr};
	if (wspThis.expired() || CHECK_PROMT_WSP_FAILED(spConn, wspThis))
		return;

	if (ec) {
		qDebug() << m_strRole.data() << " onSend ec " << ec.message().data();


		//m_spIO->dispatch([this, ec]() {
			//发送失败不再发送
			m_uShutdownState |= 0x10;
			m_spSocket->shutdown(boost::asio::socket_base::shutdown_send);
			std::queue<std::shared_ptr<PACKET>> empty;
			m_Queue.swap(empty);

			//还需要通知到外部
			if (m_fnOnConnStatus) {
				m_fnOnConnStatus(ec.value(), ec.message().data());
				m_fnOnConnStatus = NULL;
			}
			
			////其实不知道怎么处理比较好
			////但是我认为前面通知到外部了，这边close如果触发其他异常不会影响到通知事件重复
			//m_spSocket->close();

		//});
		return;
	}

	m_spStrand->post([this]() {

		if (0x10 == (m_uShutdownState & 0x10)) {
			std::queue<std::shared_ptr<PACKET>> empty;
			m_Queue.swap(empty);
			return;
		}

		if (!m_Queue.empty()) {
			auto spSended = m_Queue.front();
			
			qDebug() << m_strRole.data() << " flow out " << spSended->uRealLen;
			m_Queue.pop();
		}

		if (!m_Queue.empty()) {
			auto spNextSend = m_Queue.front();
			triggerSend(spNextSend);
		}
	});
}


void BaseConn::doRecv()
{
	qDebug() << m_strRole.data() << " doRecv";


	constexpr unsigned int uBufferSize = 4096;
	boost::shared_array<char>spszBuffer(new char[uBufferSize]);
	memset(spszBuffer.get(), 0, uBufferSize);

	m_spSocket->async_read_some(boost::asio::buffer(spszBuffer.get(), uBufferSize),
		//boost::asio::bind_executor(*m_spStrand, 
		std::bind(&BaseConn::onRecv, this,
			std::placeholders::_1,
			std::placeholders::_2,
			spszBuffer,
			shared_from_this()
		)//)
	);
}

void BaseConn::onRecv(\
	const boost::system::error_code& ec,
	size_t uRecvSize,
	boost::shared_array<char> spszBuff,
	std::weak_ptr<BaseConn> wspThis
)
{
	std::shared_ptr<BaseConn> spConn{ nullptr };
	if (wspThis.expired() || CHECK_PROMT_WSP_FAILED(spConn, wspThis))
		return;

	if (uRecvSize) {
		auto recvLen = static_cast<unsigned int>(uRecvSize);
		qDebug() <<  m_strRole.data() << " flow in " <<  recvLen;
		if (m_fnOnPacket)
			m_fnOnPacket(std::move(spszBuff), recvLen, ec.value(), ec.message().c_str());
	}

	if (ec) {
		qDebug() << m_strRole.data() << " onRecv ec " << ec.message().data();
		std::unique_lock<std::mutex> lck(m_mtxShutdownState);
		//如何处理好半连接 is a question
		if (!(m_uShutdownState & 0x01)) {
			m_spSocket->shutdown(boost::asio::socket_base::shutdown_receive);
			m_uShutdownState |= 0x01;
		}
		lck.unlock();
		//还需要通知到外部
		if (m_fnOnConnStatus) {
			m_fnOnConnStatus(ec.value(), ec.message().data());
			m_fnOnConnStatus = NULL;
		}

		return;
	}

	doRecv();
}