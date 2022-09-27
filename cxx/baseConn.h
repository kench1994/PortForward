#pragma once
#include <boost/asio.hpp>
#include <queue>
#include "conf.h"
class baseConn
{
public:
	using fntOnNetPacket = std::function<void(boost::shared_array<char>&&, \
		unsigned int, const int&, const char*)>;

	//����״̬�仯�¼�
	using fntOnConnStatus = std::function<void(const int&, const char*)>;

	baseConn();
	virtual ~baseConn();

	int addToSendChains(const std::shared_ptr<PACKET>& spPacket);

	void doRecv();

	virtual void stop() = 0;

private:
	void triggerSend(const std::shared_ptr<PACKET>& spPacket);

	void onSend(const boost::system::error_code& ec);

	void onRecv(const boost::system::error_code& ec,
		size_t  uRecvSize,
		boost::shared_array<char>spszBuff
	);

protected:
	using socket = boost::asio::ip::tcp::socket;

	std::atomic<_enConnStatus> m_auStaus;
	std::shared_ptr<socket> m_spSocket;
	std::shared_ptr<boost::asio::io_service::strand> m_spStrand;
	//���Ͷ���
	std::queue<std::shared_ptr<PACKET>> m_Queue;
	//����״̬�仯
	fntOnConnStatus m_fnOnConnStatus;
	//����·��
	fntOnNetPacket m_fnOnPacket;
};

