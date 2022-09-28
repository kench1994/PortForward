#include "baseConn.h"
#include "utils/io_service_pool.hpp"


baseConn::baseConn()
	: m_spSocket(nullptr)
	, m_auStaus(_enConnStatus::unconn)
	, m_fnOnPacket(NULL)
{
	auto pBlader = IO_EXCUTOR.pick_blader();
	m_spIO = pBlader->spIO;
	m_spStrand = pBlader->spStrand;
}


baseConn::~baseConn()
{

}


int baseConn::addToSendChains(const std::shared_ptr<PACKET>& spPacket)
{
	//目前连接上的状态是由外部管理的
	//frontend 直接等待 socket 转移所有者
	//backend 连接到 下游服务器后设置
	if (_enConnStatus::connected != m_auStaus.load())
		return -1;
	
	//适应 strand 队列避免锁竞争
	m_spStrand->post([this, spPacket](){
		m_Queue.push(std::move(spPacket));

		//触发发送
		if (1 == m_Queue.size())
			triggerSend(spPacket);
	});

	return 0;
}


void baseConn::triggerSend(const std::shared_ptr<PACKET>& spPacket)
{
	m_spSocket->async_send(boost::asio::buffer(spPacket->spszBuff.get(), spPacket->uRealLen),
		std::bind(&baseConn::onSend, this, std::placeholders::_1)
	);
}

void baseConn::onSend(const boost::system::error_code& ec)
{
	if (ec) {
		//写失败剩下的也不写了
		m_spStrand->dispatch([this, ec]() {
			m_spSocket->shutdown(boost::asio::socket_base::shutdown_send);
			std::queue<std::shared_ptr<PACKET>> empty;
			m_Queue.swap(empty);

			//还需要通知到外部
			if (m_fnOnConnStatus) {
				m_fnOnConnStatus(ec.value(), ec.message().data());
				m_fnOnConnStatus = NULL;
			}
			
			//其实不知道怎么处理比较好
			//但是我认为前面通知到外部了，这边close如果触发其他异常不会影响到通知事件重复
			m_spSocket->close();

		});
		return;
	}

	m_spStrand->post([this]() {

		if (!m_Queue.empty()) {
			auto spSended = m_Queue.front();
			
			fprintf(stdout, "%s flow out %u\r\n", m_szRole, spSended->uRealLen);
			m_Queue.pop();
		}

		if (!m_Queue.empty()) {
			auto spNextSend = m_Queue.front();
			triggerSend(spNextSend);
		}
	});
}


void baseConn::doRecv()
{
	constexpr unsigned int uBufferSize = 4096;
	boost::shared_array<char>spszBuffer(new char[uBufferSize]);
	memset(spszBuffer.get(), 0, uBufferSize);

	//m_abWorking = true;

	m_spSocket->async_read_some(boost::asio::buffer(spszBuffer.get(), uBufferSize),
		boost::asio::bind_executor(*m_spStrand, std::bind(&baseConn::onRecv, this,
			std::placeholders::_1,
			std::placeholders::_2,
			spszBuffer
		))
	);
}

void baseConn::onRecv(\
	const boost::system::error_code& ec,
	size_t  uRecvSize,
	boost::shared_array<char>spszBuff
)
{
	if (uRecvSize) {
		auto recvLen = static_cast<unsigned int>(uRecvSize);
		fprintf(stdout, "%s flow in %u\r\n", m_szRole, recvLen);
		if (m_fnOnPacket)
			m_fnOnPacket(std::move(spszBuff), recvLen, ec.value(), ec.message().c_str());
	}

	if (ec) {
		//如何处理好半连接 is a question
		m_spSocket->shutdown(boost::asio::socket_base::shutdown_receive);

		//还需要通知到外部
		if (m_fnOnConnStatus) {
			m_fnOnConnStatus(ec.value(), ec.message().data());
			m_fnOnConnStatus = NULL;
		}

		m_spSocket->close();
		return;
	}

	doRecv();
}
