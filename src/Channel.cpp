
#include <Channel.hpp>
#include <Server.hpp>
#include <unistd.h>
#include <sys/socket.h>

Channel::Channel(const std::string &name) : _name(name) {}

Channel::~Channel() {}

const std::string &Channel::getName() const
{
	return _name;
}

std::vector<Client *> &Channel::getClients() { return _clients; }

std::string Channel::getMembers() const
{
	std::string names;

	for (size_t i = 0; i < _clients.size(); i++)
	{
		names += _clients[i]->getNick();
		if (i + 1 < _clients.size())
			names += " ";
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
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == client)
		{
			_clients.erase(_clients.begin() + i);
		}
	}

}

Client *Channel::getClientByFd(int fd)
{
	std::vector<Client *>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if ((*it)->getFd() == fd)
			return *it;
	}
	return NULL;
}

void Channel::broadcast(Server *server, const std::string &msg, Client *sender, bool toAll)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == sender && !toAll)
			continue;
		server->queueMessage(_clients[i], msg);
	}
}

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
