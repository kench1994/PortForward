#include "relay.h"
#include <boost/bind.hpp>
#include <functional>
#include <iostream>


relay::relay(std::shared_ptr<baseConn> spFrontend, std::shared_ptr<baseConn> spBackend)
	: m_spBackend(std::move(spBackend)), m_spFrontend(std::move(spFrontend))
{

}

relay::~relay()
{
}
