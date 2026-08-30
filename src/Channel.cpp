
#include <Channel.hpp>
#include <Server.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include "iostream"

Channel::Channel(const std::string &name, Client *admin) : _name(name), _topic("No topic is set") ,_admin(admin), _inviteMode(false) {}

Channel::~Channel() {}

const std::string &Channel::getName() const
{
	return _name;
}
const std::string &Channel::getTopic() const {return(_topic);}

void Channel::setTopic(const std::string& other)
{
	this->_topic = other;
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

const Client &Channel::getAdmin(){return(*_admin);}
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
	//ADMIN DEBERIA SER UN SET! OperatorSet
	if (_admin == client)
		_admin = NULL;

}
void Channel::Inviteclient(Client *client)
{
	this->_invited.push_back(client);
}
void Channel::RemoveInvite(Client *client)
{
	if (!client)
		return;
	for (size_t i = 0; i < _invited.size(); i++)
	{
		if (_invited[i] == client)
		{
			_invited.erase(_invited.begin() + i);
		}
	}

}
Client *Channel::getInvitedbyFd(int fd)
{
	std::vector<Client *>::iterator it;
	for (it = _invited.begin(); it != _invited.end(); it++)
	{
		if ((*it)->getFd() == fd)
			return *it;
	}
	return(NULL);
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
void Channel::putDownInviteMode()
{
	this->_inviteMode = false;
}
void Channel::putUpInviteMode()
{
	this->_inviteMode = true;
}
bool Channel::inviteMode(){return(this->_inviteMode);}
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
bool Channel::IsInvited(Client* other)
{
	if(!getInvitedbyFd(other->getFd()))
		return(false);
	return(true);
}

bool Channel::isAdmin( Client& other)
{
	return(other == this->getAdmin() );
}
