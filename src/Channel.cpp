
#include "Channel.hpp"
#include <unistd.h>
#include <sys/socket.h>

Channel::Channel(const std::string &name) : _name(name) {}

Channel::~Channel() {}

std::string Channel::getName() const
{
	return _name;
}

void Channel::addClient(Client *client)
{
	_clients.push_back(client);
}

void Channel::broadcast(const std::string &msg, Client *sender)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] != sender)
			send(_clients[i]->fd, msg.c_str(), msg.size(), 0);
	}
}
