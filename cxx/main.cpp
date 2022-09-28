// ReverseProxy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "utils/io_service_pool.hpp"
#include "forwarder.h"
int main()
{
	
	utils::io_service_pool::instance().run();

	auto spForwarder = std::make_shared<forwarder>(80, "www.baidu.com", "80");
	
	spForwarder->start();

	//spForwarder->beginWork();

	getchar();
	return 0;
}

