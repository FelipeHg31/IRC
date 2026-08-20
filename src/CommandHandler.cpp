#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Server.hpp>

CommandHandler::CommandHandler()
{
	populateMap();
}

CommandHandler::CommandHandler(const CommandHandler &rhs) : _commands(rhs._commands) {}

CommandHandler &CommandHandler::operator=(const CommandHandler &rhs)
{
	if (this != &rhs)
		_commands = rhs._commands;
	return *this;
}

CommandHandler::~CommandHandler() {}

void CommandHandler::populateMap()
{
	_commands["PASS"] = &CommandHandler::PASS;
}

void CommandHandler::execute(std::string cmd, Server *server, Client *client, const std::vector<std::string> &args)
{
    std::map<std::string, Handler>::iterator it = _commands.find(cmd);
    if (it == _commands.end())
        return;
    it->second(server, client, args);
}

void CommandHandler::PASS(Server *server, Client *client, const std::vector<std::string> &args)
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