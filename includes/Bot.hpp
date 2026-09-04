#pragma once

#include <iostream>
#include <ctime>
#include <set>
#include <vector>

typedef const std::vector<std::string> &ArgsList;

class Client;

struct Poll
{
	int	score;
	std::string command;
	std::vector<std::string> args;
	std::string initiator;
	std::set<Client *> voters;
	std::time_t startTime;
};

class Bot
{
	public:
		Bot(Client *botClient);
		~Bot();

		void startPoll(const std::string &command, ArgsList args, const std::string &initiator);
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
