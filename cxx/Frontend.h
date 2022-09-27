#pragma once
#include "baseConn.h"
class Frontend : virtual public baseConn
{
public:
	Frontend();

	int initial(std::shared_ptr<socket>&&, \
	 const fntOnNetPacket&, const fntOnConnStatus&);

	~Frontend();

	void stop() override;
protected:
	void onRecv(boost::shared_array<char>spszBuff,\
		unsigned int uRecvSize, \
		const boost::system::error_code& ec
	);
private:
	std::atomic<bool> m_abWorking;
};

