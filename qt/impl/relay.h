#pragma once
#include "Backend.h"
#include "Frontend.h"
#include <mutex>

//数据流量中继
class relay
{
public:
	using fnt_OnNodeNotify = std::function<void(int, const char*)>;

	relay() = delete;
	explicit relay(std::shared_ptr<baseConn> spFrontend, std::shared_ptr<baseConn> spBackend);
	~relay();

	std::shared_ptr<baseConn> getBackend() { return m_spBackend; }

	std::shared_ptr<baseConn> getFrontend() { return m_spFrontend; }
protected:

private:
	//下游服务器
	std::shared_ptr<baseConn> m_spBackend;
	
	//上游数据入口
	std::shared_ptr<baseConn> m_spFrontend;

	//TODO
	//通知外部后端状态变化
	fnt_OnNodeNotify m_fnOnStatus;

};

