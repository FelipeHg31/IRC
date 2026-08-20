
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

const std::vector<std::string> &Message::getArgs() const { return _args; } 

void Message::PASS(Server *server, Client *client, std::vector<std::string> args)
{
	if (client->_registered)
		server->queueMessage(client, "already registered error\r\n");
	else
	{
		if (args[0] == server->_password)
			server->queueMessage(client, "Password OK\r\n");
		else
			server->queueMessage(client, "Password incorrect\r\n");
	}
}