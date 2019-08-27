#pragma once

#include "..\NetServer\Client.h"

class User 
{
private:
	std::string nickname;

public:
	template <std::size_t N>
	void setNickname(char(&_nickname)[N])
	{
		nickname = _nickname;
	}

	const std::string getNickname() { return nickname; }

	void releaseNickname()
	{
		nickname = "";
	}
};

