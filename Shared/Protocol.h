#pragma once

namespace Protocol
{
	enum PROTOCOL : unsigned short
	{
		INVALID = 0,
		LOGIN,			// �α���
		DISCONNECT,		// ��������
		CHAT,			// ä��
		END,
	};

	static const wchar_t* name[] =
	{
		L"INVALID",
		L"LOGIN",		// �α���
		L"DISCONNECT",	// ��������
		L"CHAT",		// ä��
		L"END",
	};
}