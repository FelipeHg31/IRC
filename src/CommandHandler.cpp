#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>
#include <iostream>
#include <sstream>

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
	_commands["PING"] =	&CommandHandler::PING;
	_commands["CAP"] =	&CommandHandler::CAP;
	_commands["PASS"] = &CommandHandler::PASS;
	_commands["NICK"] = &CommandHandler::NICK;
	_commands["USER"] = &CommandHandler::USER;
	_commands["QUIT"] = &CommandHandler::QUIT;
	_commands["JOIN"] = &CommandHandler::JOIN;
	_commands["ECHO"] = &CommandHandler::ECHO;
	_commands["MODE"] = &CommandHandler::MODE;
	_commands["INVITE"] = &CommandHandler::INVITE;
	_commands["TOPIC"] = &CommandHandler::TOPIC;
	_commands["PRIVMSG"] = &CommandHandler::PRIVMSG;
	_commands["WHO"] = &CommandHandler::WHO;
	_commands["NAMES"] = &CommandHandler::NAMES;
	_commands["LIST"] = &CommandHandler::LIST;
}

std::set<std::string> CommandHandler::populateRegCmds() const
{
	std::set<std::string> out;

	out.insert("PING");
	out.insert("CAP");
	out.insert("PASS");
	out.insert("USER");
	out.insert("NICK");
	out.insert("QUIT");

	return out;
}
void CommandHandler::execute(std::string cmd, Server *server, Client *client, ArgsList args)
{
	static std::set<std::string>	registrationCmds = populateRegCmds();
	std::string	nick = client->getNick().empty() ? "*" : client->getNick();

	std::map<std::string, Handler>::iterator it = _commands.find(cmd);
	if (it == _commands.end())
	{
		server->sendNumericMsg(client, "421", cmd + " :Unknown command");
		return;
	}
	if (!client->isRegistered() && registrationCmds.find(cmd) == registrationCmds.end())
	{
		server->sendNumericMsg(client, "451", ":You have not registered");
		return;
	}
	it->second(server, client, args);
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
	if(!chan->isAdmin(*client))
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
	if(!chan->isAdmin(*client))
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
void CommandHandler::MODE(Server *server, Client *client, ArgsList args)
{
	

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

	std::string chanName = args[0];
	Channel *chan = server->getChannel(chanName);

	if (!chan)
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return;
	}

	if(!chan->isAdmin(*client))
	{
		server->sendNumericMsg(client, "482", chanName + " :You're not channel operator");
		return;
	}
	std::string option[2] = {"+i", "-i"};
	size_t i = 0;
	for (; i < 2; i++)
	{
			if(option[i] == args[1])
				break;
	}
	switch (i)
	{
	case 0:
		chan->putUpInviteMode();
		break;
	case 1:
		chan->putDownInviteMode();
		break;
	default:
		server->sendNumericMsg(client, "472", args[2] + " : is unkown mode char for me");
		return;
	}
	

}
void CommandHandler::ECHO(Server *server, Client *client, ArgsList args)
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

void CommandHandler::PING(Server *server, Client *client, ArgsList args)
{
	if (args.empty())
		return;
	if (args[0] != server->getName())
		return;
	server->queueMessage(client, "PONG " + server->getName());
}

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
	if (args[0].size() > 9)
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
	if (server->getClientByNick(args[0]))
	{
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
	std::string reason = args.empty() ? "Client Quit" : args[0];

	server->removeClient(client->getFd(), reason);
}

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

	if (chan && chan->getClientByFd(client->getFd()))
		return;

	if (!chan)
		chan = server->addNewChannel(chanName, client);
	if(chan->inviteMode())
	{
		if(!chan->IsInvited(client))
		{
			server->sendNumericMsg(client, "473", chanName + " :Can't join channel (+i)");
			return ;
		}
		chan->RemoveInvite(client);			
	}
	chan->addClient(client);
	client->addChannel(chan);

	std::string joinMsg = ":" + client->getPrefix() + " JOIN :" + chanName + "\r\n";
	chan->broadcast(server, joinMsg, client, true);

	server->sendNumericMsg(client, "331", chanName + " :No topic is set");
	server->sendNumericMsg(client, "353", "= " + chanName + " :" + chan->getMembers());
	server->sendNumericMsg(client, "366", chanName + " :End of /NAMES list");
}

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
		if (chan->isAdmin(*member))
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

	server->sendNumericMsg(client, "353", "= " + chanName + " :" + chan->getMembers());
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
