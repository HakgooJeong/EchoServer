#include "stdafx.h"

#include "NetClient.h"

namespace NetClient
{
	Connection::Connection(boost::asio::io_service& io_service, funcDisconnect _diconnect)
		: io(io_service)
		, tcpSocket(io_service)
		, diconnector(_diconnect)
	{
	}

	bool Connection::connect(std::string ip_address, std::string tcp_port)
	{
		tcp::resolver resolver(io);

		boost::system::error_code ec;
		boost::asio::connect(tcpSocket, resolver.resolve({ ip_address, tcp_port }), ec);
		if (ec)
		{
			std::cout << "Connect Error : " << ec.message() << std::endl;
			return false;
		}

		session = std::make_shared<NetSession::TcpSession>(std::move(tcpSocket), 
			[](NetSession::TcpSession::ptrTcpSession& session, const char* msg)
			{
				printf("Session Release. Message[%s]\n", msg);
				session.reset();
			});
		session->run();

		return true;
	}

	void Connection::send(NetSession::Message& msg)
	{
		if (session == nullptr)
			return;

		session->do_write(msg);
	}

	void Connection::pop(std::shared_ptr<NetSession::Message>& _msg)
	{
		if (session == nullptr)
			return;

		if (session->isConnected() && session->count() > 0)
		{
			_msg = std::make_shared<NetSession::Message>(session->pop());
		}
	}

	void Connection::add(boost::function<void ()> func)
	{
		g.create_thread(func);
	}

	void Connection::run()
	{
		io.run_one();
	}

	void Connection::disconnect(const char* msg)
	{
		if (session == nullptr)
			return;

		diconnector(std::move(*this), msg);
		session->close(msg);
	}

	bool Connection::isConnected()
	{
		if (session == nullptr)
			return false;

		return session->isConnected();
	}
}