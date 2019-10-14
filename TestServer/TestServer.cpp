// TestServer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
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
				// ToKnow : �� �۾� ������ �Դϴ�.

				server.connectedClientMgr->update();
				server.disconnectClientMgr->update();
				server.serverMgr->update();

				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			}
		});

		server.connectedClientMgr->addProc([&server]() {

			// ToKnow : ��Ŷó��

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

				// ToKnow : ����ð� Ȯ��( 10�� ������ ���Ǹ��� )

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

			// ToKnow : ĳ�̽ð� ����� ��������

			std::vector<NetServer::Client::ptrClient> delete_list;

			for (std::map<__int64, NetServer::Client::ptrClient>::iterator iter = server.disconnectClientMgr->get().begin();
				iter != server.disconnectClientMgr->get().end();
				++iter)
			{
				// ToKnow : ����ð� Ȯ��( 20�� ������ ĳ�ø��� )

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

			// ToKnow : �ߺ�����/�翬�� ó��
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

			// ToKnow : �α��� ����

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

			// ToKnow : �������� ����

			ptrUser user = std::dynamic_pointer_cast<User, NetServer::Client>(client);
			user->diconnect("From Client Disconnected");
		}));

		server.procs.insert(std::make_pair<unsigned short, std::function<void(NetServer::Client::ptrClient, const void*)> >(
			Protocol::CHAT, [&server](NetServer::Client::ptrClient client, const void* data) {

			// ToKnow : ä�� ����

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

