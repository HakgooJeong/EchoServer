#pragma once

#include "Protocol.h"

#pragma pack(push, 1)

struct PKT_CS_LOGIN 
{
	char nickname[10] = { 0, };
};

struct PKT_SC_LOGIN 
{
	char nickname[10] = { 0, };
};

struct PKT_CS_DISCONNECT
{
};

struct PKT_SC_DISCONNECT 
{
};

struct PKT_CS_CHAT 
{
	int length;
};

struct PKT_SC_CHAT
{
	int length;
};
#pragma pack(pop)