#include "stdafx.h"

#include "Client.h"

namespace NetServer
{
	Client::Client(TcpSession::ptrTcpSession _session)
		: session(_session)
	{
	}

	Client::~Client()
	{
	}

	TcpSession::ptrTcpSession Client::getSession() const
	{
		return session;
	}

	void Client::setSession(TcpSession::ptrTcpSession session)
	{
		session = session;
	}

	void Client::clearSession()
	{
		session.reset();
	}

	void Client::send(const void* data)
	{
		if (nullptr == session)
		{
			return;
		}

		NetSession::Message msg(data);
		session->do_write(msg);
	}

	void Client::update()
	{
	}

	bool Client::isConnected() const
	{
		if (nullptr == session)
		{
			return false;
		}

		return session->isConnected();
	}

	__int64& Client::getKey()
	{
		return session->getKey();
	}
}