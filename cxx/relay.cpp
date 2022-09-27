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

	//�ȳ������ӵ����η�����
	//���ӿ��ܺ�ʱ��������ó��첽�¼�
	boost::system::error_code ec;
	return m_spBackend->connRemote([this](const int& nErrorCode, const char* pszErrInfo) {

		fprintf(stdout, "onBackEndStatus:%d\r\n", nErrorCode);
		switch (nErrorCode)
		{
			case 0:
				//���� working ״̬
				m_abIsWorking.store(true, std::memory_order::memory_order_relaxed);

				//���� incommer ͨ��
				m_spFrontend->doRecv();
				break;
			default:
				//һ���Ͽ����رն˿ڼ���
				stop();
				break;
		}
		//֪ͨ�ⲿ״̬�仯
		if (!m_fnOnStatus)
			return;
		m_fnOnStatus(nErrorCode, pszErrInfo);
	});
}

void relay::stop()
{
	//�رն˿ڼ���
	//m_spListener->stop();

	//�ر��м�
	m_abIsWorking.store(false, std::memory_order::memory_order_relaxed);
}


void relay::setOnBackendStatus(const fnt_OnNodeNotify& fnNotify)
{
	m_fnOnStatus = fnNotify;
}
