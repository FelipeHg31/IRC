
#pragma once

#include <string>

class Client
{
public:
	int fd;
	std::string nickname;
	std::string username;
	std::string buffer;
	bool passGiven;
	bool registered;
	
	Client(int fd);
	~Client();
};


