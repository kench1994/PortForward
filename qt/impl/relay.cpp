#include "Relay.h"
#include <boost/bind.hpp>
#include <functional>
#include <QDebug>


Relay::Relay(std::shared_ptr<BaseConn> spFrontend, std::shared_ptr<BaseConn> spBackend)
	: m_spBackend(std::move(spBackend)), m_spFrontend(std::move(spFrontend))
{

}

Relay::~Relay()
{
	qDebug() << "relay deconstruct";
}
