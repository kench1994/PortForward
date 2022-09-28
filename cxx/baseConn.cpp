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
	//Ŀǰ�����ϵ�״̬�����ⲿ�����
	//frontend ֱ�ӵȴ� socket ת��������
	//backend ���ӵ� ���η�����������
	if (_enConnStatus::connected != m_auStaus.load())
		return -1;
	
	//��Ӧ strand ���б���������
	m_spStrand->post([this, spPacket](){
		m_Queue.push(std::move(spPacket));

		//��������
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
		//дʧ��ʣ�µ�Ҳ��д��
		m_spStrand->dispatch([this, ec]() {
			m_spSocket->shutdown(boost::asio::socket_base::shutdown_send);
			std::queue<std::shared_ptr<PACKET>> empty;
			m_Queue.swap(empty);

			//����Ҫ֪ͨ���ⲿ
			if (m_fnOnConnStatus) {
				m_fnOnConnStatus(ec.value(), ec.message().data());
				m_fnOnConnStatus = NULL;
			}
			
			//��ʵ��֪����ô����ȽϺ�
			//��������Ϊǰ��֪ͨ���ⲿ�ˣ����close������������쳣����Ӱ�쵽֪ͨ�¼��ظ�
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
		//��δ���ð����� is a question
		m_spSocket->shutdown(boost::asio::socket_base::shutdown_receive);

		//����Ҫ֪ͨ���ⲿ
		if (m_fnOnConnStatus) {
			m_fnOnConnStatus(ec.value(), ec.message().data());
			m_fnOnConnStatus = NULL;
		}

		m_spSocket->close();
		return;
	}

	doRecv();
}
