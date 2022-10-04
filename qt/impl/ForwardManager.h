#pragma once
#include "Forwarder.h"
#include <map>
class ForwardManager
{
public:
	ForwardManager() {}

	~ForwardManager() {}

    //void delForward();

 private:
    std::mutex m_mtxForward;
	std::map<std::string, std::shared_ptr<forwarder>> m_mapForwards;
};