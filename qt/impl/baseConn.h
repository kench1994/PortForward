#pragma once
#include <boost/asio.hpp>
#include <queue>
#include "conf.h"
class BaseConn
	: public std::enable_shared_from_this<BaseConn>
{
public:
	using fntOnNetPacket = std::function<void(boost::shared_array<char>&&, \
		unsigned int, const int&, const char*)>;

	using fntOnConnStatus = std::function<void(const int&, const char*)>;

	//连接状态变化
	fntOnConnStatus m_fnOnConnStatus;


	BaseConn();

	virtual ~BaseConn();

	int addToSendChains(const std::shared_ptr<PACKET>& spPacket);

	void doRecv();

	virtual void stop() = 0;

	void setRole(const char* pszRole);

	void shutdown(const unsigned int u);
private:

	void onSend(const boost::system::error_code& ec, const std::shared_ptr<PACKET>& spPacket);

	void onRecv(const boost::system::error_code& ec,
		size_t  uRecvSize,
		boost::shared_array<char>spszBuff
	);

	std::string m_strRole;
protected:
	//0位接收,1位发送,置1表示关闭
	unsigned int m_uShutdownState;

	using socket = boost::asio::ip::tcp::socket;

	std::atomic<_enConnStatus> m_auStaus;
	std::shared_ptr<socket> m_spSocket;

	std::shared_ptr<boost::asio::io_service> m_spIO;
	std::shared_ptr<boost::asio::io_service::strand> m_spStrand;
	//发送队列
	std::queue<std::shared_ptr<PACKET>> m_Queue;
	//数据路由
	fntOnNetPacket m_fnOnPacket;

	void triggerSend(const std::shared_ptr<PACKET>& spPacket);
};

