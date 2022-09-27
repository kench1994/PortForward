#pragma once
#include "Backend.h"
#include "Frontend.h"
#include <mutex>

//���������м�
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


	//���η�����
	std::shared_ptr<Backend> m_spBackend;
	
	//�����������
	std::shared_ptr<Frontend> m_spFrontend;


	//֪ͨ�ⲿ���״̬�仯
	fnt_OnNodeNotify m_fnOnStatus;

};

