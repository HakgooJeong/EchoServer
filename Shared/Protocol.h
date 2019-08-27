#pragma once

namespace Protocol
{
	enum PROTOCOL : unsigned short
	{
		INVALID = 0,
		LOGIN,			// 로그인
		DISCONNECT,		// 연결종료
		CHAT,			// 채팅
		END,
	};

	static const wchar_t* name[] =
	{
		L"INVALID",
		L"LOGIN",		// 로그인
		L"DISCONNECT",	// 연결종료
		L"CHAT",		// 채팅
		L"END",
	};
}