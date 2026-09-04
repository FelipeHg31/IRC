#include <Bot.hpp>
#include <Client.hpp>

const std::string Bot::BOT_NAME = "KimJongBot";

Bot::Bot(Client *botClient) : _botClient(botClient), _activePoll(NULL) {}

Bot::~Bot() { delete _activePoll; }

bool Bot::isPollActive() const { return _activePoll != NULL; }

unsigned int Bot::getPollTimeout() { return POLL_DURATION; }

std::string Bot::getBotName() { return Bot::BOT_NAME; }

void Bot::startPoll(const std::string &command, ArgsList args, const std::string &initiator)
{
	if (isPollActive())
		return;

	_activePoll = new Poll();
	_activePoll->score = 0;
	_activePoll->command = command;
	_activePoll->args = args;
	_activePoll->initiator = initiator;
	_activePoll->startTime = std::time(NULL);
}

Bot::PollStatus Bot::getStatus() const
{
    PollStatus s;
    s.active = isPollActive();
    if (!s.active)
        return s;

    s.command = _activePoll->command;
    s.target = _activePoll->args.empty() ? "" : _activePoll->args.back();
    s.score = _activePoll->score;

    std::time_t elapsed = std::time(NULL) - _activePoll->startTime;
    int remaining = POLL_DURATION - static_cast<int>(elapsed);
    s.secondsLeft = remaining > 0 ? remaining : 0;

    return s;
}

bool Bot::vote(Client *voter, bool yes)
{
	if (!isPollActive())
		return false;
	if (_activePoll->voters.find(voter) != _activePoll->voters.end())
		return false;
	_activePoll->voters.insert(voter);
	if (yes)
		_activePoll->score++;
	return true;
}

void Bot::removeVoter(Client *client)
{
	if (!isPollActive())
		return;
	_activePoll->voters.erase(client);
}

Bot::PollResult Bot::checkVoteTimeout(size_t members, std::string &cmdOut, std::vector<std::string> &args)
{
	
	if (!isPollActive())
		return POLL_NONE;
	if (members == 0)
	{
		delete _activePoll;
		_activePoll = NULL;
		return POLL_FAILED;
	}
	if (std::time(NULL) - _activePoll->startTime < POLL_DURATION)
		return POLL_PENDING;

	bool passed = _activePoll->score > static_cast<int>(members / 2);

	cmdOut = _activePoll->command;
	args = _activePoll->args;

	delete _activePoll;
	_activePoll = NULL;

	return passed ? POLL_PASSED : POLL_FAILED;
}

int Bot::getRemainingTime() const
{
    if (!isPollActive())
        return -1;
    std::time_t elapsed = std::time(NULL) - _activePoll->startTime;
    int remainingSec = POLL_DURATION - static_cast<int>(elapsed);
    if (remainingSec <= 0)
        return 0;
    return remainingSec * 1000;
}
