#include "baseConn.h"



baseConn::baseConn()
	: m_spSocket(nullptr)
	, m_spStrand(nullptr)
	, m_auStaus(_enConnStatus::unconn)
	, m_fnOnPacket(NULL)
{
}


baseConn::~baseConn()
{
}


int baseConn::addToSendChains(const std::shared_ptr<PACKET>& spPacket)
{
	//if (_enConnStatus::connected != m_auStaus.load())
	//	return -1;

	//TODO:stop
	//auto spThis = shared_from_this();
	m_spStrand->post([this, spPacket]()
	{
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
		//通知失败,断开连接,清空队列
		return;
	}

	m_spStrand->post([this]() {


		if (!m_Queue.empty()) {
			auto spSended = m_Queue.front();
			
			fprintf(stdout, "sendTi :%u\r\n", spSended->uRealLen);
			m_Queue.pop();
		}
		//Read_Lock rStopStatusLock(m_rwmutStoped);
		//if (m_bStoped)
		//	return;

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
		std::bind(&baseConn::onRecv, this,
			std::placeholders::_1,
			std::placeholders::_2,
			spszBuffer
		)
	);
}

void baseConn::onRecv(\
	const boost::system::error_code& ec,
	size_t  uRecvSize,
	boost::shared_array<char>spszBuff
)
{
	if (m_fnOnPacket) 
		m_fnOnPacket(std::move(spszBuff), static_cast<unsigned int>(uRecvSize), ec.value(), ec.message().c_str());
	

	//TODO: 连接异常处理
	if (ec) {
		//m_abWorking = false;
		return;
	}

	doRecv();
}
