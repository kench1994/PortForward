#pragma once
#include "relay.h"
#include "LckFreeIncId.h"
#include <boost/utility/string_view.hpp>
#include <boost/asio.hpp>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

class forwarder
{
public:
	using socket = boost::asio::ip::tcp::socket;
	using fntOnIncommer = std::function<void(const std::shared_ptr<socket>&)>;

	forwarder() = delete;

	explicit forwarder(const unsigned int uPort, const char* pszHost, const char* pszPort);

	~forwarder();

	int start();

	void stop();

	unsigned int getPort() const;

protected:
	int beginListen();
	
	void onListen(\
		std::shared_ptr<boost::asio::ip::tcp::socket> spSocket, 
		const boost::system::error_code& ec
	);

private:
	std::unique_ptr<utils::LckFreeIncId<unsigned int>> m_spIdGen;

	unsigned int m_uPort;
	std::atomic<bool> m_abOnListen;
	std::shared_ptr<boost::asio::ip::tcp::acceptor>m_spAcceptor;

	std::mutex m_mtxRelays;
	std::unordered_map<unsigned int, std::shared_ptr<relay>> m_mapRelays;

	//���η�������Ϣ
	std::shared_ptr<Backend::NodeInfo> m_spNodeInfo;
};

