#include <Server.hpp>

void Server::tryRegistration(Client	*client)
{
	if (client->isRegistered())
		return;
	if (client->getNick().empty() || client->getUser().empty())
		return;
	if (!client->isPassGiven())
		return;
	client->setRegistered();

	sendNumericMsg(client, "001", ":Bienvenido al Internet Relay Network "
		+ client->getPrefix());
	sendNumericMsg(client, "002", ":Tu host es " + _name
		+ ", ejectuando version 1.0");
	sendNumericMsg(client, "003", ":Este servidor se creó " + _creationDate);
	sendNumericMsg(client, "004", "CONECTADO! YABBADABBADOOO!");
}

const ChannelMap &Server::getChannels() const { return _channels; }

Channel *Server::getChannel(const std::string &name)
{
	ChannelMap::iterator	it;
	it = _channels.find(name);
	if (it != _channels.end())
		return it->second;
	else
		return NULL;
}

Channel *Server::addNewChannel(const std::string &name, Client *admin)
{
	Channel	*out = new Channel(name, admin, _botClient);
	_channels[name] = out;

	return out;
}

void Server::removeChannel(const std::string &name)
{
	Channel *chan = getChannel(name);
	if (chan)
		chan->setToDelete();
}

void Server::removePendingChannels()
{
	ChannelMap::iterator it = _channels.begin();
	while (it != _channels.end())
	{
		if (it->second->isPendingDelete())
		{
			delete it->second;
			_channels.erase(it++);
		}
		else
			it++;
	}
}

static char irc_tolower(char c)
{
	switch (c)
	{
		case '[': return '{';
		case ']': return '}';
		case '\\': return '|';
		case '~': return '^';
		default:
			return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
}

std::string Server::irc_to_lower(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
		str[i] = irc_tolower(str[i]);
	return str;
}

Client *Server::getClientByNick(const std::string &nick)
{
	ClientMap::iterator it;
	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if (irc_to_lower(it->second->getNick()) == irc_to_lower(nick))
			return it->second;
	}
	return NULL;
}

void Server::checkPolls()
{
	ChannelMap::iterator it;
	for (it = _channels.begin(); it != _channels.end(); it++)
	{
		Channel *chan = it->second;
		std::string cmd;
		std::vector<std::string> args;

		Bot::PollResult res = chan->getBot().checkVoteTimeout(
			chan->getClients().size(), cmd, args);

		if (res == Bot::POLL_PASSED)
		{
			std::string notice = ":" + _botClient->getPrefix()
				+ " NOTICE " + chan->getName() + " :Vote passed, executing " + cmd + "\r\n";
			chan->broadcast(this, notice, NULL, true);
				
			_botClient->getOutBuf().clear();
			_cmdHandler.execute(cmd, this, _botClient, args);
				
			if (!_botClient->getOutBuf().empty())
			{
				std::string failNotice = ":" + _botClient->getPrefix()
					+ " NOTICE " + chan->getName()
					+ " :Vote failed (user left channel or disconnected before vote end)\r\n";
				chan->broadcast(this, failNotice, NULL, true);
			}
			_botClient->getOutBuf().clear();
		}
		else if (res == Bot::POLL_FAILED)
		{
			std::string notice = ":" + _botClient->getPrefix()
				+ " NOTICE " + chan->getName() + " :Vote failed\r\n";
			chan->broadcast(this, notice, NULL, true);
		}
	}
	removePendingChannels();
}