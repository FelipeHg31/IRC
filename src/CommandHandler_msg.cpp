#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>

static void PrivmsgChannel(Server* server, Client *client, const std::vector<std::string> &args)
{
	const std::string &chanName = args[0];

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

	const std::string reply(server->formatMessage("PRIVMSG", *client, chanName, msg ));
	chan->broadcast(server, reply,client, false);
}

static void Privmsgclient(Server* server, Client *client, const std::vector<std::string> &args)
{
	const std::string &target = args[0];

	Client *clientTarget = server->getClientByNick(target);
	if(!clientTarget)
	{
		server->sendNumericMsg(client, "401", " :No such nick");
		return ;
	}
	std::string msg(args[1]);
	for(size_t i = 2; i < args.size(); i++)
	{
		msg += " ";
		msg += args[i];
	}

	const std::string reply(server->formatMessage("PRIVMSG", *client, target, msg));
	server->queueMessage(clientTarget, reply);
}

void CommandHandler::PRIVMSG(Server *server, Client *client, ArgsList args)
{
	if(args.size() < 2)
	{
		server->sendNumericMsg(client, "461", "PRIVMSG:Not enough parameters");
		return;
	}

	const std::string &target = args[0];

	if(target[0] == '#')
		PrivmsgChannel(server, client, args);
	else
		Privmsgclient(server, client, args);
}