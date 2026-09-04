#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>

void CommandHandler::WHO(Server *server, Client *client, ArgsList args)
{
	if (args.empty())
	{
		server->sendNumericMsg(client, "315", "* :End of WHO list");
		return;
	}

	const std::string &chanName = args[0];
	Channel *chan = server->getChannel(chanName);

	if (!chan || !chan->getClientByFd(client->getFd()))
	{
		server->sendNumericMsg(client, "315", chanName + " :End of WHO list");
		return;
	}

	std::vector<Client *> &members = chan->getClients();
	for (size_t i = 0; i < members.size(); i++)
	{
		Client *member = members[i];
		std::string flags = "H";
		if (chan->isAdmin(member))
			flags += "@";

		std::string line = chanName + " " + member->getUser() + " " + member->getHost() + " "
			+ server->getName() + " " + member->getNick() + " " + flags + " :0 " + member->getRnam();

		server->sendNumericMsg(client, "352", line);
	}

	server->sendNumericMsg(client, "315", chanName + " :End of WHO list");
}

void CommandHandler::NAMES(Server *server, Client *client, ArgsList args)
{
	if (args.empty())
		return;

	const std::string &chanName = args[0];
	Channel *chan = server->getChannel(chanName);

	if (!chan)
	{
		server->sendNumericMsg(client, "366", chanName + " :End of /NAMES list");
		return;
	}

	server->sendNumericMsg(client, "353", "= " + chanName + " :@" + Bot::getBotName() + " " + chan->getMembers());
	server->sendNumericMsg(client, "366", chanName + " :End of /NAMES list");
}

void CommandHandler::LIST(Server *server, Client *client, ArgsList args)
{
	(void)args;

	const std::map<std::string, Channel *> &channels = server->getChannels();
	std::map<std::string, Channel *>::const_iterator it;

	for (it = channels.begin(); it != channels.end(); it++)
	{
		Channel *chan = it->second;
		std::ostringstream oss;
		oss << chan->getClients().size();

		server->sendNumericMsg(client, "322", chan->getName() + " " + oss.str() + " :" + chan->getTopic());
	}

	server->sendNumericMsg(client, "323", ":End of /LIST");
}

void CommandHandler::PING(Server *server, Client *client, ArgsList args)
{
	std::string token = args.empty() ? server->getName() : args[0];

	server->queueMessage(client, ":" + server->getName()
		+ " PONG " + server->getName() + " :" + token + "\r\n");
}
