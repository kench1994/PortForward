#pragma once
#include "baseConn.h"


class Backend : virtual public baseConn
{
public:
	//节点信息
	typedef struct tagNodeInfo {
		tagNodeInfo(std::string host, std::string port, unsigned int inter)
			: strHost(host), strPort(port)
		{}
		std::string strHost;
		std::string strPort;
	}NodeInfo;

	Backend();

	~Backend();

	int initial(const std::shared_ptr<NodeInfo>&, const fntOnNetPacket&, const fntOnConnStatus&);

	void stop() override;

	int connRemote();

protected:

	void onConned(std::shared_ptr<socket>spSocket, const boost::system::error_code ec);

private:
	std::shared_ptr<NodeInfo> m_spNodeInfo;
};

