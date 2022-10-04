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
private:
};

