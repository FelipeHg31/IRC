
#pragma once

#include <string>

class Client
{
	private:
		std::string	_outBuffer;
		std::string _inBuffer;
	public:
		int fd;
		std::string _nickname;
		std::string _username;

		bool _passGiven;
		bool _registered;

		std::string	&getInBuf();
		std::string	&getOutBuf();

		Client(int fd);
		~Client();
};


