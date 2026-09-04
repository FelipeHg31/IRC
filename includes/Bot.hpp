#pragma once

#include <iostream>
#include <ctime>
#include <map>
#include <vector>

typedef const std::vector<std::string> &ArgsList;

class Client;

struct Poll
{
	std::string command;
	std::vector<std::string> args;
	std::string initiator;
	std::map<Client *, int> votes;
	std::time_t startTime;
};

class Bot
{
	public:
		Bot(Client *botClient);
		~Bot();

		void startPoll(const std::string &cmd, ArgsList args, const std::string &initiator);
		bool vote(Client *voter, bool yes);
		bool isPollActive() const;
		void removeVoter(Client *client);
	private:
		Bot();
		Bot(const Bot &rhs);
		Bot &operator==(const Bot &rhs) const;
		Client	*_botClient;
		Poll	*_activePoll;
};
