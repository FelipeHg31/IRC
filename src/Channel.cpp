
#include <Channel.hpp>
#include <unistd.h>
#include <sys/socket.h>

Channel::Channel(const std::string &name) : _name(name) {}

Channel::~Channel() {}

const std::string &Channel::getName() const
{
	return _name;
}

std::vector<Client *> &Channel::getClients() { return _clients; }

void Channel::addClient(Client *client)
{
	_clients.push_back(client);
	client->_channels.insert(this);
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
			//broadcast msg
		}
	}

}

Client *Channel::getClientByFd(int fd)
{
	std::vector<Client *>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if ((*it)->fd == fd)
			return *it;
	}
	return NULL;
}

void Channel::broadcast(const std::string &msg, Client *sender)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] != sender)
			send(_clients[i]->fd, msg.c_str(), msg.size(), 0);
	}
}
