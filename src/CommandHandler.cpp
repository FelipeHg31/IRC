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
	_commands["NICK"] = &CommandHandler::NICK;
	_commands["ECHO"] = &CommandHandler::ECHO;
}

void CommandHandler::execute(std::string cmd, Server *server, Client *client, const std::vector<std::string> &args)
{
    std::map<std::string, Handler>::iterator it = _commands.find(cmd);
    if (it == _commands.end())
        return;
    it->second(server, client, args);
}
void CommandHandler::ECHO(Server *server, Client *client, const std::vector<std::string> &args)
{
	std::string reply;
	for (size_t i = 0; i < args.size(); i++)
		reply += args[i] + " ";
	reply += "\r\n";
	server->queueMessage(client, reply);
}

void CommandHandler::PASS(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (client->_passGiven)
	{
		server->queueMessage(client, server->formatError("462", client->_nickname.empty() ? "*" : client->_nickname, ":Connection already registered"));
		return;
	}
	if (args.size() != 1)
	{
		server->queueMessage(client, server->formatError("461", client->_nickname.empty() ? "*" : client->_nickname, "PASS :Syntax error"));
		return;
	}
	if (args[0] == server->_password)
	{
		client->_passGiven = true;
	}
}

void CommandHandler::NICK(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (args.size() != 1 || args[0].empty())
	{
		server->queueMessage(client, server->formatError("461", client->_nickname.empty() ? "*" : client->_nickname, "NICK :Syntax error"));
		return;
	}
	if (args[0].size() > 9)
	{
		server->queueMessage(client, server->formatError("432", client->_nickname.empty() ? "*" : client->_nickname, args[0] + " :Nickname too long, max 9 characters"));
		return;
	}
	if (server->getClientByNick(args[0]))
	{
		server->queueMessage(client, server->formatError("433", client->_nickname.empty() ? "*" : client->_nickname, args[0] + " :Nickname already in use"));
		return;
	}
}