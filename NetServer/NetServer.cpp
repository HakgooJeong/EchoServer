#include "stdafx.h"

#include "NetServer.h"
#include "ClientMgr.h"

namespace NetServer
{
	Server::Server(boost::asio::io_service& io_service, short tcp_port, short udp_port, funcCreate _create)
		:io(io_service),
		acceptor(io_service, tcp::endpoint(tcp::v4(), tcp_port)),
		tcpSocket(io_service),
		create(_create)
	{
		connectedClientMgr = std::make_shared<ClientMgr>();
		disconnectClientMgr = std::make_shared<ClientMgr>();
		serverMgr = std::make_shared<ClientMgr>();

		SYSTEM_INFO si;
		GetSystemInfo(&si);

		for (int i = 0; i < (int)si.dwNumberOfProcessors; ++i)
		{
			g.create_thread([this, &io_service]()
			{
				io_service.run();
			});
		}

		start_accept();
	}

    void Server::start_accept()
    {
		acceptor.async_accept(tcpSocket,
            [this](boost::system::error_code ec)
        {
            static __int64 key = 0;

            if (!ec)
            {
                std::shared_ptr<TcpSession> new_session = std::make_shared<TcpSession>(std::move(tcpSocket), 
					[](TcpSession::ptrTcpSession& session, const char* msg)
					{
						printf("Session Release. Message[%s]\n", msg);
						session.reset();
					},
					++key);
				connectedClientMgr->add(create(new_session));
				new_session->run();

                printf("New Session Connected [Key:%lld]\n", key);

				start_accept();
            }
        });
    }

	void Server::add(std::function<void()> func)
	{
		g.create_thread(func);
	}
}