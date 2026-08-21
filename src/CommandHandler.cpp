#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>
#include <iostream>

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
	_commands["USER"] = &CommandHandler::USER;
	_commands["ECHO"] = &CommandHandler::ECHO;
	_commands["JOIN"] = &CommandHandler::JOIN;
}

void CommandHandler::execute(std::string cmd, Server *server, Client *client, const std::vector<std::string> &args)
{
	std::set<std::string>	registrationCmds;
	registrationCmds.insert("CAP");
	registrationCmds.insert("PASS");
	registrationCmds.insert("USER");
	registrationCmds.insert("NICK");

	std::map<std::string, Handler>::iterator it = _commands.find(cmd);
	if (it == _commands.end())
	{
		server->queueMessage(client, server->formatNumeric("421", client->getNick().empty() ? "*" : client->getNick(), cmd + " :Unknown command"));
		return;
	}
	if (!client->isRegistered() && registrationCmds.find(cmd) == registrationCmds.end())
	{
		server->queueMessage(client, server->formatNumeric("451", "*", ":You have not registered"));
		return;
	}
	it->second(server, client, args);
}
void CommandHandler::ECHO(Server *server, Client *client, const std::vector<std::string> &args)
{
	std::string reply;
	for (size_t i = 0; i < args.size(); i++)
		reply += args[i] + " ";
	reply += "\r\n";
	std::cout << "ECHO from user -> " << client->getNick() << " : " << reply;
	server->queueMessage(client, reply);
}

void CommandHandler::announceNickChange(Server *server, Client *client, const std::string &oldNick, const std::string &newNick)
{
	std::string notice = ":" + oldNick + " NICK :" + newNick + "\r\n";
	std::set<Channel *> &channels = client->getChannels();

	if (channels.empty())
	{
		server->queueMessage(client, notice);
		return;
	}

	//std::set<Channel *>::iterator it;
	//for (it = channels.begin(); it != channels.end(); it++)
	//	(*it)->broadcastAll(server, notice);
}

bool CommandHandler::isValidNickChar(char c, bool isFirst)
{
	if (isFirst)
		return (isalpha(static_cast<unsigned char>(c)) || c == '_');
	return (isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-');
}

void CommandHandler::PASS(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (client->isRegistered())
	{
		server->queueMessage(client, server->formatNumeric("462", client->getNick().empty() ? "*" : client->getNick(), ":You may not reregister"));
		return;
	}
	if (client->isPassGiven())
		return;
	if (args.size() != 1)
	{
		server->queueMessage(client, server->formatNumeric("461", client->getNick().empty() ? "*" : client->getNick(), "PASS :Not enough parameters"));
		return;
	}
	if (args[0] != server->getPass())
	{
		server->queueMessage(client, server->formatNumeric("464", client->getNick().empty() ? "*" : client->getNick(), ":Password incorrect"));
		return;
	}
	client->setPassGiven();
}

void CommandHandler::NICK(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (args.size() < 1 || args[0].empty())
	{
		server->queueMessage(client, server->formatNumeric("431", "*", ":No nickname given"));
		return;
	}

	if (args[0].size() > 9)
	{
		server->queueMessage(client, server->formatNumeric("432", client->getNick().empty() ? "*" : client->getNick(), args[0] + " :Erroneous nickname"));
		return;
	}
	for (size_t i = 0; i < args[0].size(); i++)
	{
		if (!isValidNickChar(args[0][i], i == 0))
		{
			server->queueMessage(client, server->formatNumeric("432", client->getNick().empty() ? "*" : client->getNick(), args[0] + " :Erroneous nickname"));
			return;
		}
	}
	if (server->getClientByNick(args[0]))
	{
		server->queueMessage(client, server->formatNumeric("433", client->getNick().empty() ? "*" : client->getNick(), args[0] + " :Nickname is already in use"));
		return;
	}

	std::string oldNick = client->getNick();
	client->setNick(args[0]);

	if (client->isRegistered())
		announceNickChange(server, client, oldNick, args[0]);

	server->tryRegistration(client);
}

void CommandHandler::USER(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (client->isRegistered())
	{
		server->queueMessage(client, server->formatNumeric("462", client->getNick().empty() ? "*" : client->getNick(), ":You may not reregister"));
		return;
	}
	if (!client->getUser().empty())
		return;
	if (args.size() < 4)
	{
		server->queueMessage(client, server->formatNumeric("461", client->getNick().empty() ? "*" : client->getNick(), "USER :Not enough parameters"));
		return;
	}

	const std::string &username = args[0];

	if (username.empty())
	{
		server->queueMessage(client, server->formatNumeric("461", client->getNick().empty() ? "*" : client->getNick(), "USER :Erroneous username"));
		return;
	}
	for (size_t i = 0; i < username.size(); i++)
	{
		if (!isValidNickChar(username[i], false))
		{
			server->queueMessage(client, server->formatNumeric("461", client->getNick().empty() ? "*" : client->getNick(), "USER :Erroneous username"));
			return;
		}
	}

	client->setUser(username);
	client->setRnam(args[3]);

	server->tryRegistration(client);
}

void CommandHandler::JOIN(Server *server, Client *client, const std::vector<std::string> &args)
{
	client->setRegistered();
	if (!client->isRegistered())
	{
		server->queueMessage(client, server->formatNumeric("451", client->getNick().empty() ? "*" : client->getNick(), " :Connection not registered"));
		return;
	}
	if (args.size() < 1)
	{
		server->queueMessage(client, server->formatNumeric("461", client->getNick().empty() ? "*" : client->getNick(), " JOIN :Syntax error"));
		return;
	}

	Channel *chan = server->getChannel(args[0]);
	// implementar en futuro lo de JOIN canal1,canal2 y que los canales empiezan siempre con #
	if (chan && !chan->getClientByFd(client->getFd()))
	{
		chan->addClient(client);
	}
	else
	{
		chan = server->addNewChannel(args[0]);
		chan->addClient(client);
		//new -> makeclientadmin?
	}
	chan->broadcast(server, "teeeeeest!", client);
}
