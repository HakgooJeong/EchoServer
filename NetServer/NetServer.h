#pragma once

#include <boost/asio.hpp>

#include "..\NetSession\NetSession.h"

namespace NetServer
{
    using boost::asio::ip::tcp;
    using boost::asio::ip::udp;

	using namespace NetSession;

	class ClientMgr;
	class Client;

    class Server : public std::enable_shared_from_this<Server>
    {
	public:

		typedef std::shared_ptr<Client>	ptrClient;
		typedef std::function<ptrClient(TcpSession::ptrTcpSession)> funcCreate;

    public:

		Server(boost::asio::io_service& io_service, short tcp_port, short udp_port, funcCreate _create);
		void join() { g.join_all(); }
		void add(std::function<void()> func);

    private:

        void start_accept();

        // tcp
		boost::asio::io_service& io;
        tcp::acceptor acceptor;
        tcp::socket tcpSocket;

		// thread
		boost::thread_group g;
		boost::mutex mutex;

		funcCreate		create;

	public:

		// manager
		std::shared_ptr<ClientMgr> serverMgr;			// ToKnow : 멀티서버를 위한 서버 매니저. 추후 개발
		std::shared_ptr<ClientMgr> connectedClientMgr;
		std::shared_ptr<ClientMgr> disconnectClientMgr;

		// proc
		std::map<unsigned short, std::function<void(ptrClient, const void*)> > procs;
    };
}