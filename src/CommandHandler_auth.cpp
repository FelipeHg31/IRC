#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>

void CommandHandler::CAP(Server *server, Client *client, ArgsList args)
{
	if (args.empty())
		return;

	if (args[0] == "LS")
	{
		const std::string reply(":" + server->getName()
			+ " CAP " + client->getNick() + " LS :\r\n");
		server->queueMessage(client, reply);
	}
}

void CommandHandler::PASS(Server *server, Client *client, ArgsList args)
{
	if (client->isRegistered())
	{
		server->sendNumericMsg(client, "462", ":You may not reregister");
		return;
	}
	if (client->isPassGiven())
		return;
	if (args.size() != 1)
	{
		server->sendNumericMsg(client, "461", "PASS :Not enough parameters");
		return;
	}
	if (args[0] != server->getPass())
	{
		server->sendNumericMsg(client, "464", ":Password incorrect");
		return;
	}
	client->setPassGiven();
}

void CommandHandler::NICK(Server *server, Client *client, ArgsList args)
{
	if (args.size() < 1 || args[0].empty())
	{
		server->sendNumericMsg(client, "431", ":No nickname given");
		return;
	}
	if (args[0].size() > 9 || Server::irc_to_lower(args[0]) == Server::irc_to_lower(server->getBot()->getNick()))
	{
		server->sendNumericMsg(client, "432", args[0] + " :Invalid nickname");
		return;
	}
	for (size_t i = 0; i < args[0].size(); i++)
	{
		if (!isValidNickChar(args[0][i], i == 0))
		{
			server->sendNumericMsg(client, "432", args[0] + " :Invalid nickname");
			return;
		}
	}

	Client *existing = server->getClientByNick(args[0]); 
	
	if (existing)
	{
		if (existing != client)
			server->sendNumericMsg(client, "433", args[0] + " :Nickname already in use");
		return;
	}

	std::string oldNick = client->getNick();
	client->setNick(args[0]);

	if (client->isRegistered())
		announceNickChange(server, client, oldNick, args[0]);

	server->tryRegistration(client);
}

void CommandHandler::USER(Server *server, Client *client, ArgsList args)
{
	std::string	nick = client->getNick().empty() ? "*" : client->getNick();

	if (client->isRegistered())
	{
		server->sendNumericMsg(client, "462", ":You may not reregister");
		return;
	}
	if (!client->getUser().empty())
		return;
	if (args.size() < 4)
	{
		server->sendNumericMsg(client, "461", "USER :Not enough parameters");
		return;
	}

	const std::string &username = args[0];

	if (username.empty())
	{	
		server->sendNumericMsg(client, "461", "USER :Erroneous username");
		return;
	}
	for (size_t i = 0; i < username.size(); i++)
	{
		if (!isValidNickChar(username[i], false))
		{
			server->sendNumericMsg(client, "461", "USER :Erroneous username");
			return;
		}
	}

	client->setUser(username);
	client->setRnam(args[3]);

	server->tryRegistration(client);
}

void CommandHandler::QUIT(Server *server, Client *client, ArgsList args)
{
	(void)server;
	
	std::string reason = args.empty() ? "Client Quit" : args[0];

	client->setToDisconnect(reason);
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
