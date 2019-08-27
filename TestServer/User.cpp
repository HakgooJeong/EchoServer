#include "stdafx.h"

#include "User.h"

User::User(TcpSession::ptrTcpSession _session, User::funcDisconnect _diconnector)
	: Client(_session)
	, diconnector(_diconnector)
{
}

User::~User()
{
}

void User::diconnect(const char* msg)
{
	diconnector(shared_from_this(), msg);
	getSession()->close(msg);
}

void User::shutDown(const char* msg)
{
	diconnector(shared_from_this(), msg);
	getSession()->close(msg);
	getSession()->SessionTime0();
}
