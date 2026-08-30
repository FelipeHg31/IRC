
#pragma once

#include <string>
#include <vector>
#include <Server.hpp>

class Client;

class Message
{
	private:
		std::string	_cmd;
		std::vector<std::string>	_args;
		Message();
	public:
		Message(const std::string &line);
		Message(const Message &rhs);
		Message &operator=(const Message &rhs);
		~Message();
		const std::string	&getCmd() const;
		ArgsList getArgs() const;
};
