
#include <Message.hpp>
#include <Client.hpp>
#include <Server.hpp>
#include <sstream>
#include <sys/socket.h>

Message::Message() {}

Message::~Message() {}

Message::Message(const std::string &line)
{
	std::istringstream iss(line);
	bool	first = true;
	std::string	token;

	while (iss >> token)
	{
		if (first)
		{
			_cmd = token;
			first = false;
		}
		else if (token[0] == ':')
		{
			std::string	end;
			std::getline(iss, end);
			token.erase(0, 1);
			_args.push_back(token + end);
			break;
		}
		else
			_args.push_back(token);
	}
}

Message::Message(const Message &rhs) : _cmd(rhs._cmd), _args(rhs._args) {}

Message &Message::operator=(const Message &rhs)
{
	if (this != &rhs)
	{
		_cmd = rhs._cmd;
		_args = rhs._args;
	}
	return *this;
}

const std::string &Message::getCmd() const { return _cmd; }

ArgsList Message::getArgs() const { return _args; } 

