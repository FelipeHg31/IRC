#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>
#include <iostream>
#include <sstream>

CommandHandler::CommandHandler()
{
	populateMap();
	populatePreRegCmds();
}

CommandHandler &CommandHandler::operator=(const CommandHandler &rhs)
{
	if (this != &rhs)
		_commands = rhs._commands;
	return *this;
}

CommandHandler::~CommandHandler() {}

void CommandHandler::populateMap()
{
	_commands["CAP"] =	&CommandHandler::CAP;
	_commands["PASS"] = &CommandHandler::PASS;
	_commands["NICK"] = &CommandHandler::NICK;
	_commands["USER"] = &CommandHandler::USER;
	_commands["QUIT"] = &CommandHandler::QUIT;
	_commands["JOIN"] = &CommandHandler::JOIN;
	_commands["MODE"] = &CommandHandler::MODE;
	_commands["TOPIC"] = &CommandHandler::TOPIC;
	_commands["INVITE"] = &CommandHandler::INVITE;
	_commands["PART"] = &CommandHandler::PART;
	_commands["PRIVMSG"] = &CommandHandler::PRIVMSG;
	_commands["WHO"] = &CommandHandler::WHO;
	_commands["NAMES"] = &CommandHandler::NAMES;
	_commands["LIST"] = &CommandHandler::LIST;
	_commands["PING"] =	&CommandHandler::PING;
}

void CommandHandler::populatePreRegCmds()
{
	_preRegCommands.insert("PING");
	_preRegCommands.insert("CAP");
	_preRegCommands.insert("PASS");
	_preRegCommands.insert("USER");
	_preRegCommands.insert("NICK");
	_preRegCommands.insert("QUIT");
}
void CommandHandler::execute(std::string cmd, Server *server, Client *client, ArgsList args)
{
	std::map<std::string, Handler>::iterator it = _commands.find(cmd);
	if (it == _commands.end())
	{
		server->sendNumericMsg(client, "421", cmd + " :Unknown command");
		return;
	}
	if (!client->isRegistered() && _preRegCommands.find(cmd) == _preRegCommands.end())
	{
		server->sendNumericMsg(client, "451", ":You have not registered");
		return;
	}
	it->second(server, client, args);
}
