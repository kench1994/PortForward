#pragma once
#include "Relay.h"
#include "LckFreeIncId.h"
#include "utils/twisted_io_rate.hpp"
#include <boost/utility/string_view.hpp>
#include <boost/asio.hpp>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

class Forwarder
{
public:
	using socket = boost::asio::ip::tcp::socket;
	using fntOnIncommer = std::function<void(const std::shared_ptr<socket>&)>;
	
	/**
	 * 通知后台连接数变化
	 * unsigned int 监听端口
	 * unsigned int 连接数
	**/
	using fntNotifyForwardConnCnt = std::function<void(unsigned int, unsigned int)>;

	Forwarder() = delete;

	explicit Forwarder(const unsigned int uPort, const char* pszHost, const char* pszPort);

	~Forwarder();

	int start();

	void stop();

	unsigned int getPort() const;

	std::shared_ptr<Relay> delRelay(const unsigned int id);

	void setNotifyConnCnt(const fntNotifyForwardConnCnt&);

	void setNotifyRating(const utils::twisted_io_rate::fntNotifyTwistedRating&);
protected:
	int beginListen();
	
	void onListen(\
		std::shared_ptr<boost::asio::ip::tcp::socket> spSocket, 
		const boost::system::error_code& ec
	);

private:
	fntNotifyForwardConnCnt m_fnNotifyConnCnt;

	std::shared_ptr<Forwarder> m_spForwarder;

	std::unique_ptr<utils::LckFreeIncId<unsigned int>> m_spIdGen;

	unsigned int m_uPort;
	std::atomic<bool> m_abOnListen;
	std::shared_ptr<boost::asio::ip::tcp::acceptor>m_spAcceptor;

	std::mutex m_mtxRelays;
	std::unordered_map<unsigned int, std::shared_ptr<Relay>> m_mapRelays;

	//下游服务器信息
	std::shared_ptr<Backend::NodeInfo> m_spNodeInfo;

	//流速计算
	std::shared_ptr<utils::twisted_io_rate> m_spRater;
};

