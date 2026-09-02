#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>

static void sendToChannel(Server* server, Client *client, ArgsList args, bool notice)
{
	const std::string &chanName = args[0];
	const std::string cmdName = notice ? "NOTICE" : "PRIVMSG";

	Channel *chan = server->getChannel(chanName);
	if(!chan)
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return;
	}
	std::string msg(args[1]);
	for(size_t i = 2; i < args.size(); i++)
	{
		msg += " ";
		msg += args[i];
	}

	const std::string reply(server->formatMessage(cmdName, *client, chanName, msg ));
	chan->broadcast(server, reply,client, false);
}

static void sendToClient(Server* server, Client *client, ArgsList args, bool notice)
{
	const std::string &target = args[0];
	const std::string cmdName = notice ? "NOTICE" : "PRIVMSG";

	Client *clientTarget = server->getClientByNick(target);
	if(!clientTarget)
	{
		server->sendNumericMsg(client, "401", target + " :No such nick");
		return ;
	}
	std::string msg(args[1]);
	for(size_t i = 2; i < args.size(); i++)
	{
		msg += " ";
		msg += args[i];
	}

	const std::string reply(server->formatMessage(cmdName, *client, target, msg));
	server->queueMessage(clientTarget, reply);
}

static void sendMessage(Server *server, Client *client, ArgsList args, bool notice)
{
	if (args.size() < 2)
	{
		if (!notice)
			server->sendNumericMsg(client, "461", "PRIVMSG :Not enough parameters");
		return;
	}

	const std::string &target = args[0];

	if (target[0] == '#')
		sendToChannel(server, client, args, notice);
	else
		sendToClient(server, client, args, notice);
}

void CommandHandler::PRIVMSG(Server *server, Client *client, ArgsList args)
{
	sendMessage(server, client, args, false);
}

void CommandHandler::NOTICE(Server *server, Client *client, ArgsList args)
{
	sendMessage(server, client, args, true);
}