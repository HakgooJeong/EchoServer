// TestServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
#include "stdafx.h"

#include "..\NetServer\NetServer.h"
#include "..\NetServer\ClientMgr.h"

#include "..\Shared\Packet.h"
#include "..\Shared\Protocol.h"

#include "User.h"

using namespace NetSession;

int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: async_echo_server <port> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

		NetServer::Server& server = NetServer::Server(io_service, std::atoi(argv[1]), std::atoi(argv[2]),
			[](TcpSession::ptrTcpSession session) -> NetServer::Client::ptrClient
			{ 
				return NetServer::Client::ptrClient(new User(session,	[](NetServer::Client::ptrClient& client, const char* msg)
																		{
																			ptrUser user = std::dynamic_pointer_cast<User, NetServer::Client>(client);

																			PKT_SC_DISCONNECT disconnect;

																			NetSession::Message packet;
																			packet << disconnect;
																			packet.close(Protocol::DISCONNECT);

																			user->send(packet.get());
																			printf("Sended Disconnect\n");
																		}));
			});

		server.add([&server]() {

			while (true)
			{
				// ToKnow : 주 작업 쓰레드 입니다.

				server.connectedClientMgr->update();
				server.disconnectClientMgr->update();
				server.serverMgr->update();

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		});

		server.connectedClientMgr->addProc([&server]() {

			// ToKnow : 패킷처리

			std::vector<NetServer::Client::ptrClient> delete_connect_list;
			std::vector<NetServer::Client::ptrClient> add_disconnect_list;

			NetServer::Client::ptrClient newOne = nullptr;

			for (std::map<__int64, NetServer::Client::ptrClient>::iterator iter = server.connectedClientMgr->get().begin();
				iter != server.connectedClientMgr->get().end();
				++iter)
			{
				NetServer::Client::ptrClient impl = (*iter).second;

				if (impl->getSession()->count() > 0)
				{
					impl->getSession()->refreshSessionTime();

					NetSession::Message&& msg = impl->getSession()->pop();
					server.procs[msg.cmd()](impl, msg.body());
				}

				// ToKnow : 연결시간 확인( 10초 지나면 세션만료 )

				if (GetTickCount64() - (*iter).second->getSession()->getSessionTime() > 30000)
				{
					delete_connect_list.push_back((*iter).second);
					add_disconnect_list.push_back((*iter).second);

					ptrUser user = std::dynamic_pointer_cast<User, NetServer::Client>((*iter).second);
					user->diconnect("Disconnected By SessionTime");
				}
			}

			for each (auto var in delete_connect_list)
			{
				server.connectedClientMgr->remove(var);
			}

			for each (auto var in add_disconnect_list)
			{
				server.disconnectClientMgr->add(var);
			}
		});

		server.disconnectClientMgr->addProc([&server]() {

			// ToKnow : 캐싱시간 만료된 유저제거

			std::vector<NetServer::Client::ptrClient> delete_list;

			for (std::map<__int64, NetServer::Client::ptrClient>::iterator iter = server.disconnectClientMgr->get().begin();
				iter != server.disconnectClientMgr->get().end();
				++iter)
			{
				// ToKnow : 연결시간 확인( 20초 지나면 캐시만료 )

				if (GetTickCount64() - (*iter).second->getSession()->getSessionTime() > 60000)
				{
					delete_list.push_back((*iter).second);
				}
			}

			for each (auto var in delete_list)
			{
				server.disconnectClientMgr->remove(var);
				printf("Cache removed guid:%s\n", var->getGuid().c_str());
			}
		});

		server.procs.insert(std::make_pair<unsigned short, std::function<void(NetServer::Client::ptrClient, const void*)> >(
			Protocol::LOGIN, [&server](NetServer::Client::ptrClient client, const void* data) {

			ptrUser user = std::dynamic_pointer_cast<User, NetServer::Client>(client);

			PKT_CS_LOGIN* recved = (PKT_CS_LOGIN*)data;

			// ToKnow : 중복접속/재연결 처리
			std::string nickname(recved->nickname);

			NetServer::Client::ptrClient oldOne = server.connectedClientMgr->find(nickname);
			if (nullptr != oldOne)
			{
				ptrUser oldUser = std::dynamic_pointer_cast<User, NetServer::Client>(oldOne);
				oldUser->shutDown("Dulpicated Remove");
			}

			if (oldOne == nullptr)
			{
				oldOne = server.disconnectClientMgr->find(nickname);
				if (nullptr != oldOne)
				{
					ptrUser oldUser = std::dynamic_pointer_cast<User, NetServer::Client>(oldOne);
					oldUser->shutDown("Dulpicated Remove");
				}
			}

			// ToKnow : 로그인 로직

			PKT_SC_LOGIN send;
			memcpy_s(send.nickname, 10, recved->nickname, 10);

			NetSession::Message packet;
			packet << send;
			packet.close(Protocol::LOGIN);

			user->send(packet.get());

			user->setNickname(recved->nickname);
			user->setGuid(recved->nickname);
		}));

		server.procs.insert(std::make_pair<unsigned short, std::function<void(NetServer::Client::ptrClient, const void*)> >(
			Protocol::DISCONNECT, [](NetServer::Client::ptrClient client, const void* data) {

			// ToKnow : 접속종료 로직

			ptrUser user = std::dynamic_pointer_cast<User, NetServer::Client>(client);
			user->diconnect("From Client Disconnected");
		}));

		server.procs.insert(std::make_pair<unsigned short, std::function<void(NetServer::Client::ptrClient, const void*)> >(
			Protocol::CHAT, [&server](NetServer::Client::ptrClient client, const void* data) {

			// ToKnow : 채팅 로직

			ptrUser user = std::dynamic_pointer_cast<User, NetServer::Client>(client);

			if ( true == user->getNickname().empty() )
			{
				user->shutDown("NotLoginUser");
				return;
			}

			PKT_CS_CHAT* recved = (PKT_CS_CHAT*)data;

			PKT_SC_CHAT send;
			memcpy_s(send.nickname, 10, user->getNickname().c_str(), 10);
			send.length = recved->length;

			printf("recv:%s\n", (char*)(recved+1));

			NetSession::Message packet;
			packet << send;
			packet.write((char*)(recved + 1), (unsigned short)send.length);
			packet.close(Protocol::CHAT);

			server.connectedClientMgr->broadcast(packet.get());
		}));

		server.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

