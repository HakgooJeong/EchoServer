#pragma once

#include <cstdlib>
#include <iostream>

#include <boost/asio.hpp>

#include "..\NetSession\NetSession.h"

namespace NetClient
{
	using boost::asio::ip::tcp;
	using boost::asio::ip::udp;

	class Connection
	{
	public:
		typedef std::function<void(Connection&, const char*)> funcDisconnect;

	public:
		Connection(boost::asio::io_service& io_service, funcDisconnect diconnect);

		bool connect(std::string ip_address, std::string tcp_port);
		void join() { g.join_all(); }

		void send(NetSession::Message& msg);
		void pop(std::shared_ptr<NetSession::Message>& _msg);

		void add(boost::function<void()> func);
		void run();
		void disconnect(const char* mag);
		bool isConnected();

	private:
		boost::asio::io_service& io;

		// tcp
		tcp::socket tcpSocket;
		std::shared_ptr<NetSession::TcpSession> session;

		// thread
		boost::thread_group g;
		boost::mutex mutex;

		funcDisconnect diconnector;
	};
}