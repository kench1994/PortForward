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
	explicit relay(std::shared_ptr<Frontend> spFrontend, std::shared_ptr<Backend> spBackend);
	~relay();

	int start();

	void stop();

	//TODO:Node
	void setOnBackendStatus(const fnt_OnNodeNotify& fnNotify);

protected:

private:
	std::atomic<bool> m_abIsWorking;


	//下游服务器
	std::shared_ptr<Backend> m_spBackend;
	
	//上游数据入口
	std::shared_ptr<Frontend> m_spFrontend;


	//通知外部后端状态变化
	fnt_OnNodeNotify m_fnOnStatus;

};

