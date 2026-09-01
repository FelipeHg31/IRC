#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>

void CommandHandler::JOIN(Server *server, Client *client, ArgsList args)
{
	if (args.size() < 1)
	{
		server->sendNumericMsg(client, "461", "JOIN :Not enough parameters");
		return;
	}

	const std::string &chanName = args[0];

	if (!Channel::isValidChannelName(chanName))
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return;
	}

	Channel *chan = server->getChannel(chanName);

	if (chan)
	{
		if (chan->getClientByFd(client->getFd()))
			return;

		std::string passGiven = (args.size() > 1) ? args[1] : "";

		if (!checkJoinPermissions(server, client, chan, passGiven))
			return;
	}
	else
		chan = server->addNewChannel(chanName, client);

	chan->addClient(client);
	client->addChannel(chan);

	std::string joinMsg = ":" + client->getPrefix() + " JOIN :" + chanName + "\r\n";
	chan->broadcast(server, joinMsg, client, true);

	if (chan->getTopic().empty())
		server->sendNumericMsg(client, "331", chanName + " :No topic is set");
	else
		server->sendNumericMsg(client, "332", chanName + " :" + chan->getTopic());

	server->sendNumericMsg(client, "353", "= " + chanName + " :" + chan->getMembers());
	server->sendNumericMsg(client, "366", chanName + " :End of /NAMES list");
}

static void selectUpMode(size_t option,Channel *chan, ArgsList args, size_t *actualArg)
{
	switch (option)
	{
		case 0:
			chan->putUpInviteMode();
			break;
		case 1:
			chan->lockTopic();
			break;
		case 2:
			chan->lockPassword();
			chan->setPassword(args[*(actualArg++)]);
			break;
		default:
			return;
	}
}
static void selectDownMode(size_t option,Channel *chan)
{
	switch (option)
	{
		case 0:
			chan->putUpInviteMode();
			break;
		case 1:
			chan->unlockTopic();
			break;
		case 2:
			chan->unlockPassword();
			break;
		default:
			return;
	}
}

void CommandHandler::MODE(Server *server, Client *client, ArgsList args)
{
	
	std::string chanName = args[0];

	if (args.size() < 2)
	{
		server->sendNumericMsg(client, "461", "MODE:Not enough parameters");
		return;
	}

	if (!client->isRegistered())
	{
		server->sendNumericMsg(client, "462", ":You may not reregister");
		return;
	}
	if (!Channel::isValidChannelName(chanName))
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return;
	}
	Channel *chan = server->getChannel(chanName);

	if(!chan->isAdmin(client))
	{
		server->sendNumericMsg(client, "482", chanName + " :You're not channel operator");
		return;
	}
	std::string option[5] = {"i", "t", "k", "o", "l"};
	char 	ActivationState = '+';
	size_t actualArg  = 2;
	for( size_t i= 0 ; i <  args[1].size(); i++)
	{
		size_t j = 0;
		for(; j < 5; j++)
		{
			if(args[1][i] == option[j][0])
				break;
		}
		if(j == 5 && (args[1][i] == '+' || args[1][i] == '-'))
		{
			ActivationState = args[1][i];
			continue;
		}
		else if(j == 5)
		{
			server->sendNumericMsg(client, "472", args[1][i] + " : is unkown mode char for me");
			return;
		}
		else if(actualArg >= args.size() && j  >= 2 )
		{
			server->sendNumericMsg(client, "461","MODE : Not enought parametres");
			return ;
		}
		if(ActivationState == '+')
			selectUpMode(j, chan, args, &actualArg);
		else
			selectDownMode(j, chan);
	}
}

void CommandHandler::TOPIC(Server *server, Client *client, ArgsList args)
{
	if(args.size() < 2)
	{
		server->sendNumericMsg(client, "461", "TOPIC:Not enough parameters");
		return;
	}

	const std::string &chanName = args[0];

	if (!Channel::isValidChannelName(chanName))
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return;
	}
	Channel *chan = server->getChannel(chanName);
	if(!chan)
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return;
	}
	if (!chan->getClientByFd(client->getFd()))
		return;
	if(!chan->isAdmin(client))
	{
		server->sendNumericMsg(client, "482", chanName + " :You're not channel operator");
		return;
	}
	std::string topic(args[1]);
	
	for(size_t i = 2; i < args.size(); i++)
	{
		topic += " ";
		topic += args[i];
	}
	chan->setTopic(topic);
	std::cout << server->formatMessage("TOPIC", *client, chanName, topic ) << std::endl;
	chan->broadcast(server, server->formatMessage("TOPIC", *client, chanName, topic ),client, true);
}

void CommandHandler::INVITE(Server *server, Client *client, ArgsList args)
{
	if (args.size() < 2)
	{
		server->sendNumericMsg(client, "461", "INVITE:Not enough parameters");
		return;
	}

	const std::string &chanName = args[0];

	if (!Channel::isValidChannelName(chanName))
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return;
	}


	Channel *chan = server->getChannel(chanName);
	if(!chan)
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return;
	}
	if(!chan->isAdmin(client))
	{
		server->sendNumericMsg(client, "482", chanName + " :You're not channel operator");
		return;
	}
	Client *invited = server->getClientByNick(args[1]);
	if(!invited)
	{
		server->sendNumericMsg(client, "401", " :No such nick");
	}
	if(chan->IsInvited(invited))
		return;
	chan->Inviteclient(invited);
}

bool CommandHandler::checkJoinPermissions(Server *server,
						Client *client, Channel *channel, const std::string &password)
{
	if (channel->getUserLimit() != 0 && channel->getClients().size() >= channel->getUserLimit())
	{
		server->sendNumericMsg(client, "471", channel->getName() + " :Cannot join channel (+l)");
		return false;
	}
	if (channel->hasPassword())
	{
		if (password != channel->getPassword())
		{
			server->sendNumericMsg(client, "475", channel->getName() + " :Cannot join channel (+k)");
			return false;
		}
	}
	if (channel->inviteMode())
	{
		if (!channel->IsInvited(client))
		{
			server->sendNumericMsg(client, "473", channel->getName() + " :Can't join channel (+i)");
			return false;
		}
		channel->RemoveInvite(client);
	}
	return true;
}