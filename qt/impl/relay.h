#pragma once
#include "Backend.h"
#include "Frontend.h"
#include <mutex>

//数据流量中继
class Relay
{
public:
	using fnt_OnNodeNotify = std::function<void(int, const char*)>;

	Relay() = delete;
	explicit Relay(std::shared_ptr<BaseConn> spFrontend, std::shared_ptr<BaseConn> spBackend);
	~Relay();

	std::shared_ptr<BaseConn> getBackend() { return m_spBackend; }

	std::shared_ptr<BaseConn> getFrontend() { return m_spFrontend; }
protected:

private:
	//下游服务器
	std::shared_ptr<BaseConn> m_spBackend;
	
	//上游数据入口
	std::shared_ptr<BaseConn> m_spFrontend;

	//TODO
	//通知外部后端状态变化
	fnt_OnNodeNotify m_fnOnStatus;

};

