
#pragma once

#include <string>
#include <vector>

class Client;
class Server;

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
		const std::vector<std::string>	&getArgs() const;
};
