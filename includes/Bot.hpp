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
		enum PollResult
		{
			POLL_NONE,
			POLL_PENDING,
			POLL_PASSED,
			POLL_FAILED
		};
		struct PollStatus
		{
			bool		active;
			std::string	command;
			std::string	target;
			int			score;
			int			secondsLeft;
		};
		Bot(Client *botClient);
		~Bot();

		static unsigned int getPollTimeout();
		bool isTargetNick(const std::string &nick) const;
		void renameTarget(const std::string &newNick);

		void startPoll(const std::string &command, ArgsList args, const std::string &initiator);
		bool vote(Client *voter, bool yes);
		bool isPollActive() const;
		PollStatus getStatus() const;
		int getRemainingTime() const;
		void removeVoter(Client *client);
		PollResult checkVoteTimeout(size_t members, std::string &cmdOut, std::vector<std::string> &args);
		static std::string getBotName();
	private:
		Bot();
		Bot(const Bot &rhs);
		Bot &operator=(const Bot &rhs) const;

		static const unsigned int POLL_DURATION = 30;
		static const std::string BOT_NAME;
		Client	*_botClient;
		Poll	*_activePoll;
};
