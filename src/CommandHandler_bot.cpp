#include <CommandHandler.hpp>
#include <Server.hpp>

static void announcePollStart(Server *server, Channel *chan, const std::string &command, const std::string &target, const std::string &initiator)
{
	std::ostringstream oss;
	oss << "Vote started by " << initiator << ": " << command << " " << target
		<< " (" << Bot::getPollTimeout() << " seconds to vote, use KIM YES/NO)";

	std::string notice = ":" + server->getBot()->getPrefix()
		+ " NOTICE " + chan->getName() + " :" + oss.str() + "\r\n";
	chan->broadcast(server, notice, NULL, true);
}

static bool checkBotArgs(Server *server, Client *client, ArgsList args, size_t minSize)
{
	if (args.size() < minSize)
	{
		server->sendNumericMsg(client, "461", "KIM :Not enough parameters");
		return false;
	}
	return true;
}

static Channel *validateBotChannel(Server *server, Client *client, const std::string &chanName)
{
	Channel *chan = server->getChannel(chanName);
	if (!chan)
	{
		server->sendNumericMsg(client, "403", chanName + " :No such channel");
		return NULL;
	}
	if (!chan->getClientByFd(client->getFd()))
	{
		server->sendNumericMsg(client, "442", chanName + " :You're not on that channel");
		return NULL;
	}
	return chan;
}

static void startKickPoll(Server *server, Client *client, Channel *chan, const std::string &chanName, ArgsList args)
{
	if (!checkBotArgs(server, client, args, 3))
		return;
	if (chan->getBot().isPollActive())
	{
		server->sendNumericMsg(client, "666", chanName + " :A vote is already in progress");
		return;
	}

	std::vector<std::string> kickArgs;
	kickArgs.push_back(chanName);
	kickArgs.push_back(args[2]);

	chan->getBot().startPoll("KICK", kickArgs, client->getNick());
	announcePollStart(server, chan, "KICK", args[2], client->getNick());
}

static void startModePoll(Server *server, Client *client, Channel *chan, const std::string &chanName, ArgsList args, bool promote)
{
	if (!checkBotArgs(server, client, args, 3))
		return;
	if (chan->getBot().isPollActive())
	{
		server->sendNumericMsg(client, "666", chanName + " :A vote is already in progress");
		return;
	}

	std::vector<std::string> modeArgs;
	modeArgs.push_back(chanName);
	modeArgs.push_back(promote ? "+o" : "-o");
	modeArgs.push_back(args[2]);

	chan->getBot().startPoll("MODE", modeArgs, client->getNick());
	announcePollStart(server, chan, promote ? "PROMOTE" : "DEMOTE", args[2], client->getNick());
}

static void castVote(Server *server, Client *client, Channel *chan, bool positive)
{
	if (!chan->getBot().vote(client, positive))
	{
		server->sendNumericMsg(client, "666", chan->getName() + " :No active vote or already voted");
		return;
	}
	Bot::PollStatus s = chan->getBot().getStatus();
	std::ostringstream oss;
	oss << "Voting to " << s.command << " " << s.target << ", "
		<< s.score << " YES, " << s.secondsLeft << " SECONDS LEFT";

	std::string notice = ":" + server->getBot()->getPrefix()
		+ " NOTICE " + chan->getName() + " :" + oss.str() + "\r\n";
	chan->broadcast(server, notice, NULL, true);
}

static void botHelpMessage(Server *server, Client *client)
{
	std::ostringstream oss;
	oss << Bot::getPollTimeout();
	std::string timeout = oss.str();
	server->queueMessage(client, ":" + server->getBot()->getPrefix()
		+ " NOTICE " + client->getTarget() + " :Bot commands: KIM KICK #chan <nick> | KIM PROMOTE #chan <nick> | KIM DEMOTE #chan <nick> | KIM YES #chan | KIM NO #chan | KIM HELP\r\n");
	server->queueMessage(client, ":" + server->getBot()->getPrefix()
		+ " NOTICE " + client->getTarget() + " :Starting a vote requires being on the channel; a vote passes with a majority of channel members voting YES within " + timeout +" seconds.\r\n");
}

void CommandHandler::KIM(Server *server, Client *client, ArgsList args)
{
	if (args.empty())
		return;

	const std::string &sub = args[0];

	if (sub == "HELP")
	{
		botHelpMessage(server, client);
		return;
	}
	if (!checkBotArgs(server, client, args, 2))
		return;

	const std::string &chanName = args[1];

	Channel *chan = validateBotChannel(server, client, chanName);
	if (!chan)
		return;

	if (sub == "KICK")
		startKickPoll(server, client, chan, chanName, args);
	else if (sub == "PROMOTE")
		startModePoll(server, client, chan, chanName, args, true);
	else if (sub == "DEMOTE")
		startModePoll(server, client, chan, chanName, args, false);
	else if (sub == "YES")
		castVote(server, client, chan, true);
	else if (sub == "NO")
		castVote(server, client, chan, false);
	else
		server->sendNumericMsg(client, "666", sub + " :Unknown KIM subcommand");
}
