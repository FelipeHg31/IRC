#include <Bot.hpp>
#include <Client.hpp>

Bot::Bot(Client *botClient) : _botClient(botClient), _activePoll(NULL) {}

Bot::~Bot() { delete _activePoll; }

bool Bot::isPollActive() const { return _activePoll != NULL; }

void Bot::startPoll(const std::string &command, ArgsList args, const std::string &initiator)
{
	if (isPollActive())
		return;

	_activePoll = new Poll();
	_activePoll->command = command;
	_activePoll->args = args;
	_activePoll->initiator = initiator;
	_activePoll->startTime = std::time(NULL);
}

bool Bot::vote(Client *voter, bool positive)
{
	if (!isPollActive())
		return false;
	if (_activePoll->votes.find(voter) != _activePoll->votes.end())
		return false;

	_activePoll->votes[voter] = positive ? 1 : -1;
	return true;
}

void Bot::removeVoter(Client *client)
{
	if (!isPollActive())
		return;
	_activePoll->votes.erase(client);
}