#include "relay.h"
#include <boost/bind.hpp>
#include <functional>
#include <iostream>


relay::relay(std::shared_ptr<Frontend> spFrontend, std::shared_ptr<Backend> spBackend)
	: m_spBackend(spBackend), m_spFrontend(spFrontend)
{

}

relay::~relay()
{
}

int relay::start()
{
	int nRet = -1;
	if (!m_spBackend)
		return nRet;

	//先尝试连接到下游服务器
	//连接可能耗时，因此设置成异步事件
	boost::system::error_code ec;
	return m_spBackend->connRemote([this](const int& nErrorCode, const char* pszErrInfo) {

		fprintf(stdout, "onBackEndStatus:%d\r\n", nErrorCode);
		switch (nErrorCode)
		{
			case 0:
				//设置 working 状态
				m_abIsWorking.store(true, std::memory_order::memory_order_relaxed);

				//开启 incommer 通道
				m_spFrontend->doRecv();
				break;
			default:
				//一旦断开，关闭端口监听
				stop();
				break;
		}
		//通知外部状态变化
		if (!m_fnOnStatus)
			return;
		m_fnOnStatus(nErrorCode, pszErrInfo);
	});
}

void relay::stop()
{
	//关闭端口监听
	//m_spListener->stop();

	//关闭中继
	m_abIsWorking.store(false, std::memory_order::memory_order_relaxed);
}


void relay::setOnBackendStatus(const fnt_OnNodeNotify& fnNotify)
{
	m_fnOnStatus = fnNotify;
}
