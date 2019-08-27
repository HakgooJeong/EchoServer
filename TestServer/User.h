#pragma once

#include "..\NetServer\Client.h"

using namespace NetSession;

class User : public NetServer::Client
{
public:

	typedef std::function<void(NetServer::Client::ptrClient&, const char*)> funcDisconnect;

public:

	User(TcpSession::ptrTcpSession session, funcDisconnect diconnector);
	virtual ~User();

private:

	std::string nickname;
	funcDisconnect diconnector;

public:

	template <std::size_t N>
	void setNickname(char (&_nickname)[N])
	{
		nickname = _nickname;
	}

	const std::string getNickname() { return nickname.c_str(); }

	void diconnect(const char* msg);	// ToKnow : ���Ǹ� ����
	void shutDown(const char* msg);		// ToKnow : ���� �� ĳ�ø���
};

typedef std::shared_ptr<User> ptrUser;

