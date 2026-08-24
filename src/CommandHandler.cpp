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
	_commands["CAP"] =	&CommandHandler::CAP;
	_commands["PASS"] = &CommandHandler::PASS;
	_commands["NICK"] = &CommandHandler::NICK;
	_commands["USER"] = &CommandHandler::USER;
	_commands["QUIT"] = &CommandHandler::QUIT;
	_commands["JOIN"] = &CommandHandler::JOIN;
	_commands["ECHO"] = &CommandHandler::ECHO;
}

std::set<std::string> CommandHandler::populateRegCmds() const
{
	std::set<std::string> out;

	out.insert("CAP");
	out.insert("PASS");
	out.insert("USER");
	out.insert("NICK");
	out.insert("QUIT");

	return out;
}

void CommandHandler::execute(std::string cmd, Server *server, Client *client, const std::vector<std::string> &args)
{
	static std::set<std::string>	registrationCmds = populateRegCmds();
	std::string	nick = client->getNick().empty() ? "*" : client->getNick();

	std::map<std::string, Handler>::iterator it = _commands.find(cmd);
	if (it == _commands.end())
	{
		server->queueMessage(client, server->formatNumeric("421", nick, cmd + " :Unknown command"));
		return;
	}
	if (!client->isRegistered() && registrationCmds.find(cmd) == registrationCmds.end())
	{
		server->queueMessage(client, server->formatNumeric("451", nick, ":You have not registered"));
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
	std::string oldPrefix = oldNick + "!" + client->getUser() + "@" + client->getHost();
	std::string notice = ":" + oldPrefix + " NICK :" + newNick + "\r\n";
	std::set<Client *> peers = client->getChannelPeers();

	if (peers.empty())
	{
		server->queueMessage(client, notice);
		return;
	}

	std::set<Client *>::iterator it;
	for (it = peers.begin(); it != peers.end(); it++)
		server->queueMessage(*it, notice);
}

bool CommandHandler::isValidNickChar(char c, bool isFirst)
{
	if (isFirst)
		return (isalpha(static_cast<unsigned char>(c)) || c == '_');
	return (isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-');
}

void CommandHandler::CAP(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (args.empty())
		return;

	std::string nick = client->getNick().empty() ? "*" : client->getNick();
	if (args[0] == "LS")
		server->queueMessage(client, ":ircserv CAP " + nick + " LS :\r\n");
}

void CommandHandler::PASS(Server *server, Client *client, const std::vector<std::string> &args)
{
	std::string	nick = client->getNick().empty() ? "*" : client->getNick();

	if (client->isRegistered())
	{
		server->queueMessage(client, server->formatNumeric("462", nick, ":You may not reregister"));
		return;
	}
	if (client->isPassGiven())
		return;
	if (args.size() != 1)
	{
		server->queueMessage(client, server->formatNumeric("461", nick, "PASS :Not enough parameters"));
		return;
	}
	if (args[0] != server->getPass())
	{
		server->queueMessage(client, server->formatNumeric("464", nick, ":Password incorrect"));
		return;
	}
	client->setPassGiven();
}

void CommandHandler::NICK(Server *server, Client *client, const std::vector<std::string> &args)
{
	std::string	nick = client->getNick().empty() ? "*" : client->getNick();
	if (args.size() < 1 || args[0].empty())
	{
		server->queueMessage(client, server->formatNumeric("431", nick, ":No nickname given"));
		return;
	}
	if (args[0].size() > 9)
	{
		server->queueMessage(client, server->formatNumeric("432", nick, args[0] + " :Erroneous nickname"));
		return;
	}
	for (size_t i = 0; i < args[0].size(); i++)
	{
		if (!isValidNickChar(args[0][i], i == 0))
		{
			server->queueMessage(client, server->formatNumeric("432", nick, args[0] + " :Erroneous nickname"));
			return;
		}
	}
	if (server->getClientByNick(args[0]))
	{
		server->queueMessage(client, server->formatNumeric("433", nick, args[0] + " :Nickname is already in use"));
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
	std::string	nick = client->getNick().empty() ? "*" : client->getNick();

	if (client->isRegistered())
	{
		server->queueMessage(client, server->formatNumeric("462", nick, ":You may not reregister"));
		return;
	}
	if (!client->getUser().empty())
		return;
	if (args.size() < 4)
	{
		server->queueMessage(client, server->formatNumeric("461", nick, "USER :Not enough parameters"));
		return;
	}

	const std::string &username = args[0];

	if (username.empty())
	{
		server->queueMessage(client, server->formatNumeric("461", nick, "USER :Erroneous username"));
		return;
	}
	for (size_t i = 0; i < username.size(); i++)
	{
		if (!isValidNickChar(username[i], false))
		{
			server->queueMessage(client, server->formatNumeric("461", nick, "USER :Erroneous username"));
			return;
		}
	}

	client->setUser(username);
	client->setRnam(args[3]);

	server->tryRegistration(client);
}

void CommandHandler::QUIT(Server *server, Client *client, const std::vector<std::string> &args)
{
	std::string reason = args.empty() ? "Client Quit" : args[0];

	server->removeClient(client->getFd(), reason);
}

void CommandHandler::JOIN(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (args.size() < 1)
	{
		server->queueMessage(client, server->formatNumeric("461", client->getNick(), "JOIN :Not enough parameters"));
		return;
	}

	const std::string &chanName = args[0];

	if (!Channel::isValidChannelName(chanName))
	{
		server->queueMessage(client, server->formatNumeric("403", client->getNick(), chanName + " :No such channel"));
		return;
	}

	Channel *chan = server->getChannel(chanName);

	if (chan && chan->getClientByFd(client->getFd()))
		return;

	if (!chan)
		chan = server->addNewChannel(chanName);

	chan->addClient(client);
	client->addChannel(chan);

	std::string joinMsg = ":" + client->getPrefix() + " JOIN :" + chanName + "\r\n";
	chan->broadcast(server, joinMsg, client, true);

	server->queueMessage(client, server->formatNumeric("331", client->getNick(), chanName + " :No topic is set"));
	server->queueMessage(client, server->formatNumeric("353", client->getNick(), "= " + chanName + " :" + chan->getMembers()));
	server->queueMessage(client, server->formatNumeric("366", client->getNick(), chanName + " :End of /NAMES list"));
}

