
#include <Channel.hpp>
#include <Server.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include "iostream"

Channel::Channel(const std::string &name, Client *admin) : _name(name), _topic(""), _inviteMode(false), _passwordMode(false), _topicLocked(false), _userLimit(0)
{
	if (!admin)
		throw std::runtime_error("Something broke.");
	_operators.insert(admin);
}

Channel::~Channel() {}

const std::string &Channel::getName() const
{
	return _name;
}

const std::string &Channel::getPassword() const { return _password; }

void Channel::setPassword(const std::string &password) { _password = password; }

void Channel::lockPassword() { _passwordMode = true; }

void Channel::unlockPassword() { _passwordMode = false; }

unsigned int	Channel::getUserLimit() const { return _userLimit; }

bool Channel::isTopicLocked() const { return _topicLocked; }

void Channel::lockTopic() { _topicLocked = true; }

void Channel::unlockTopic() { _topicLocked = false; }

void Channel::setUserLimit(int limit)
{
	if (limit > 0 && limit < 100)
		_userLimit = limit;
}

bool Channel::hasPassword() const { return _passwordMode; }

const std::string &Channel::getTopic() const { return _topic; }

void Channel::setTopic(const std::string& other)
{
	this->_topic = other;
}
std::vector<Client *> &Channel::getClients() { return _clients; }

std::string Channel::getMembers() const
{
	std::string names;

	std::vector<Client *>::const_iterator it;
	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it != _clients.begin())
			names += " ";
		if (isAdmin(*it))
			names += "@";
		names += (*it)->getNick();
	}
	return names;
}

void Channel::addClient(Client *client)
{
	_clients.push_back(client);
}

void Channel::removeClient(Client *client)
{
	if (!client)
		return;

	std::vector<Client *>::iterator it;

	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if (*it == client)
		{
			_clients.erase(it);
			break;
		}
	}
	std::set<Client *>::iterator itOps = _operators.find(client);
	if (itOps != _operators.end())
		_operators.erase(itOps);
}
void Channel::Inviteclient(Client *client)
{
	this->_invited.insert(client);
}
void Channel::RemoveInvite(Client *client)
{
	if (!client)
		return;

	std::set<Client *>::iterator it = _invited.find(client);

	if (it != _invited.end())
		_invited.erase(it);
}
Client *Channel::getInvitedbyFd(int fd) const
{
	std::set<Client *>::const_iterator it;
	for (it = _invited.begin(); it != _invited.end(); it++)
	{
		if ((*it)->getFd() == fd)
			return *it;
	}
	return(NULL);
}
Client *Channel::getClientByFd(int fd) const
{
	std::vector<Client *>::const_iterator it;
	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if ((*it)->getFd() == fd)
			return *it;
	}
	return NULL;
}
void Channel::broadcast(Server *server, const std::string &msg, Client *sender, bool toAll)
{
	std::vector<Client *>::iterator it;

	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if (*it == sender && !toAll)
			continue;
		server->queueMessage(*it, msg);
	}
}
void Channel::putDownInviteMode()
{
	this->_inviteMode = false;
}
void Channel::putUpInviteMode()
{
	this->_inviteMode = true;
}
bool Channel::inviteMode() { return this->_inviteMode; }

bool Channel::isValidChannelName(const std::string &name)
{
	if (name.size() < 2 || name.size() > 50)
		return false;
	if (name[0] != '#')
		return false;
	for (size_t i = 1; i < name.size(); i++)
	{
		if (name[i] == ',' || name[i] == ':' || name[i] == ' ')
			return false;
	}
	return true;
}
bool Channel::IsInvited(Client *other)
{
	if(!other || !getInvitedbyFd(other->getFd()))
		return(false);
	return(true);
}

bool Channel::isAdmin(Client *client) const
{
	if (_operators.find(client) != _operators.end())
		return true;
	return false;
}

void Channel::addOperator(Client *client) { _operators.insert(client); }

void Channel::removeOperator(Client *client) { _operators.erase(client); }