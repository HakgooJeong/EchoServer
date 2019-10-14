// TestClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
#include "stdafx.h"

#include "..\NetClient\NetClient.h"

#include "..\Shared\Protocol.h"
#include "..\Shared\Packet.h"

#include "User.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
		return 1;
	}

	boost::asio::io_service io_service;

	NetClient::Connection conn(io_service,	[](NetClient::Connection& conn, const char* msg)
											{
												PKT_CS_DISCONNECT disconnect;

												NetSession::Message packet;
												packet << disconnect;
												packet.close(Protocol::DISCONNECT);

												conn.send(packet);
												printf("Sended Disconnect\n");
											});

	const char* ip = argv[1];
	const char* port = argv[2];

	printf("\'connect\': connect to server\n");
	printf("\'disconnect\': disconnect from server\n");

	User user;

	conn.add([&ip, &port, &conn, &user]() {

		// ToKnow : 에코서버 주 작업 쓰레드 입니다.

		while (true)
		{
			std::string str;
			std::getline(std::cin, str);
			size_t size = str.size();

			if (0 == str.compare("disconnect"))
			{
				conn.disconnect("From Client Disconnect");
			}
			else if (0 == str.compare("connect"))
			{
				if (false == conn.isConnected())
				{
					if (false == conn.connect(ip, port))
					{
						return 2;
					}

					user.releaseNickname();
				}
			}
			else if (true == conn.isConnected() && true == user.getNickname().empty())
			{
				PKT_CS_LOGIN login;
				memcpy_s(login.nickname, sizeof(PKT_CS_LOGIN), str.c_str(), sizeof(PKT_CS_LOGIN));

				NetSession::Message packet;
				packet << login;
				packet.close(Protocol::LOGIN);

				conn.send(packet);
			}
			else
			{
				PKT_CS_CHAT chat;
				chat.length = size;

				NetSession::Message packet;
				packet << chat;
				packet.write(str.c_str(), (unsigned short)size);
				packet.close(Protocol::CHAT);

				conn.send(packet);
			}

			std::cin.clear();
		}
	});

	conn.add([&conn]() {
		 
		// ToKnow : IO 용 쓰레드
		while (true)
		{
			if(true == conn.isConnected() )
				conn.run();

			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		}
	});

	conn.add([&conn, &user]() {

		// ToKnow : 메세지 팝업 쓰레드

		while (true)
		{
			std::shared_ptr<NetSession::Message> packet;
			conn.pop(packet);

			if (packet != nullptr)
			{
				printf("size:%d cmd:%S\n", packet->size(), Protocol::name[packet->cmd()]);
				
				switch ((Protocol::PROTOCOL)packet->cmd())
				{
				case Protocol::LOGIN:
					{
						PKT_SC_LOGIN* recved = (PKT_SC_LOGIN*)packet->body();
						user.setNickname(recved->nickname);

						printf("recv:%s\n", recved->nickname);
					}
					break;

				case Protocol::DISCONNECT:
					{
						conn.disconnect("From Server Disconnect");
					}
					break;

				case Protocol::CHAT:
					{
						PKT_SC_CHAT* recved = (PKT_SC_CHAT*)packet->body();
						
						size_t szTail = 0;
						char* msg = (char*)packet->tail(sizeof(PKT_SC_CHAT), szTail);

						printf("recv:from %s : %s\n", recved->nickname, msg);
					}
					break;

				default:
					break;
				}
			}

			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		}
	});

	conn.join();

	return 0;
}

