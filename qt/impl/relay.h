#pragma once
#include "Backend.h"
#include "Frontend.h"

//数据流量中继
class Relay
{
public:
	Relay() = delete;
	explicit Relay(std::shared_ptr<BaseConn> spFrontend, std::shared_ptr<BaseConn> spBackend);
	~Relay();

	std::shared_ptr<BaseConn> getBackend();

	std::shared_ptr<BaseConn> getFrontend();

protected:

private:
	//下游服务器
	std::shared_ptr<BaseConn> m_spBackend;
	
	//上游数据入口
	std::shared_ptr<BaseConn> m_spFrontend;

};

