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
	explicit relay(std::shared_ptr<baseConn> spFrontend, std::shared_ptr<baseConn> spBackend);
	~relay();

	int start();

	void stop();

protected:

private:
	std::atomic<bool> m_abIsWorking;


	//���η�����
	std::shared_ptr<baseConn> m_spBackend;
	
	//�����������
	std::shared_ptr<baseConn> m_spFrontend;

	//TODO
	//֪ͨ�ⲿ���״̬�仯
	fnt_OnNodeNotify m_fnOnStatus;

};

