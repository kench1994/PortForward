#include "forwarder.h"
#include "utils/io_service_pool.hpp"


forwarder::forwarder(const unsigned int uPort, const char* pszHost, const char* pszPort)
	: m_abOnListen(false)
	, m_uPort(uPort)
	, m_spIdGen(std::make_unique<utils::LckFreeIncId<unsigned int>>(0))
	, m_spNodeInfo(\
		std::make_shared<Backend::NodeInfo>(\
		pszHost, pszPort, 0)
	 )
{
}


forwarder::~forwarder()
{
}

int forwarder::start()
{
	//不重复开启端口监听
	if (m_spAcceptor || m_abOnListen.load(std::memory_order::memory_order_relaxed))
		return 0;

	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), m_uPort);

	m_spAcceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(\
		*utils::io_service_pool::instance().pick_io_service(), \
		ep, false
	);

	//端口监听器创建失败
	if (!m_spAcceptor)
		return -1;

	//开启监听
	beginListen();
	return 0;
}


void forwarder::stop()
{
	m_spAcceptor->close();
	m_abOnListen.store(false, std::memory_order::memory_order_relaxed);
}


int forwarder::beginListen()
{
	try {
		if (!m_spAcceptor || !m_spAcceptor->is_open())
			return false;

		m_abOnListen.store(true, std::memory_order::memory_order_relaxed);

		auto spSocket = std::make_shared<boost::asio::ip::tcp::socket>(m_spAcceptor->get_executor());
		m_spAcceptor->async_accept(*spSocket, \
			std::bind(&forwarder::onListen, this, spSocket, \
				std::placeholders::_1)
		);
	} catch (const std::exception&) {
		//TODO:LOG
		//e.what();
		return false;
	}
	return true;
}

void forwarder::onListen(\
	std::shared_ptr<boost::asio::ip::tcp::socket> spSocket,
	const boost::system::error_code& ec
)
{
	if (ec) {
		//告知前端异常
		return;
	}

	fprintf(stdout, "conn join\r\n");

	auto id = m_spIdGen->increase();
	auto spFrontend = std::make_shared<Frontend>();
	spFrontend->setRole("Front-end");
	auto spBackend = std::make_shared<Backend>();
	spBackend->setRole("Back-end");

	std::weak_ptr<Backend> wspBack(spBackend);
	spFrontend->initial(std::move(spSocket), [wspBack, this](boost::shared_array<char>&& spszBuff, unsigned int uBufLen, const int& nError, const char* pszErrInfo) {

		fprintf(stdout, "Front-end flow in %u\r\n", uBufLen);

		std::shared_ptr<Backend> spBackend;
		if (wspBack.expired() || CHECK_PROMT_WSP_FAILED(spBackend, wspBack))
			return;

		if (nError) {
			//TODO:通知前端
			spBackend->stop();
			return;
		}
		if (!uBufLen)
			return;

		//转发数据流量
		auto spPacket = std::make_shared<PACKET>(std::forward<boost::shared_array<char>&&>(spszBuff), uBufLen);
		spBackend->addToSendChains(spPacket);

	}, [this, id](const int& nErrorCode, const char* pszErrInfo){

		assert(nErrorCode);

		fprintf(stdout, "Front-end has disconnected\r\n");

		std::unique_lock<std::mutex> lck(m_mtxRelays);
		auto itF = m_mapRelays.find(id);
		if (itF == m_mapRelays.end())
			return;
		auto spRelay = itF->second;
		m_mapRelays.erase(itF);
		lck.unlock();

		spRelay->getBackend()->stop();
	});

	//TODO:frontend 断开 通知backend 断开
	//backend断开删除这个relay
	//前端可以统计forwarder持有的relay（连接数等信息

	//TODO：通知前端
	std::weak_ptr<Frontend> wspFront(spFrontend);
	spBackend->initial(m_spNodeInfo, [wspFront](boost::shared_array<char>&& spszBuff, unsigned int uBufLen, const int& nError, const char* pszErrInfo) {

		fprintf(stdout, "Back-end flow in %u\r\n", uBufLen);

		std::shared_ptr<Frontend> spFrontend;
		if (wspFront.expired() || CHECK_PROMT_WSP_FAILED(spFrontend, wspFront))
			return;

		if (nError) {
			spFrontend->stop();
			return;
		}
		if (!uBufLen)
			return;

		//转发数据流量
		auto spPacket = std::make_shared<PACKET>(std::forward<boost::shared_array<char>&&>(spszBuff), uBufLen);
		spFrontend->addToSendChains(spPacket);

	}, [this, id, wspFront](const int& nErrorCode, const char* pszErrInfo){
		if(0 == nErrorCode){

			fprintf(stdout, "Back-end has connected to downward server\r\n");
			//开启 incommer 通道
			std::shared_ptr<Frontend> spFrontend;
			if (wspFront.expired() || CHECK_PROMT_WSP_FAILED(spFrontend, wspFront))
				return;
			fprintf(stdout, "Front-end begin recv\r\n");
			spFrontend->doRecv();
			return;
		}

		fprintf(stdout, "Back-end has disconnected\r\n");

		//Backend断开删除这个relay
		std::unique_lock<std::mutex> lck(m_mtxRelays);
		auto itF = m_mapRelays.find(id);
		if(itF == m_mapRelays.end())
			return;
		auto spRelay = itF->second;
		m_mapRelays.erase(itF);
		lck.unlock();

		//Front-end 断开连接
		spRelay->getFrontend()->stop();
	});

	std::unique_lock<std::mutex> lck(m_mtxRelays);
	m_mapRelays[id] = std::move(std::make_shared<relay>(spFrontend, spBackend));
	lck.unlock();

	IO_EXCUTOR.pick_io_service()->post([spBackend]() {
		spBackend->connRemote();
	});

	//continue listen
	beginListen();
}

unsigned int forwarder::getPort() const
{
	return m_uPort;
}
