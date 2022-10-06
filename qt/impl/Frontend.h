#pragma once
#include "BaseConn.h"
class Frontend : virtual public BaseConn
{
public:
	Frontend();

	int initial(std::shared_ptr<socket>&&, \
	 const fntOnNetPacket&, const fntOnConnStatus&);

	~Frontend();

	void stop() override;
private:
};

