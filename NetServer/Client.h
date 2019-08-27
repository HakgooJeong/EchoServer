#pragma once

#include "..\NetSession\NetSession.h"

namespace NetServer
{
	using namespace NetSession;

	class Client : public std::enable_shared_from_this<Client>
	{
	public:
		typedef std::shared_ptr<Client>	ptrClient;

	public:
		Client(TcpSession::ptrTcpSession session);
		virtual ~Client();

		void						setSession(TcpSession::ptrTcpSession session);
		TcpSession::ptrTcpSession	getSession() const;
		void						clearSession();

		bool						isConnected() const;
		__int64&					getKey();

		std::string&				getGuid() { return guid; }
		void						setGuid(const char* _guid) { guid = _guid; }

		virtual void update();
		virtual void send(const void* data);

	private:
		TcpSession::ptrTcpSession	session;
		std::string					guid = "";
	};
}