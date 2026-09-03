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
	Channel	*out = new Channel(name, admin);
	_channels[name] = out;

	return out;
}

void Server::removeChannel(const std::string &name)
{
	ChannelMap::iterator it = _channels.find(name);
	if (it == _channels.end())
		return;
	delete it->second;
	_channels.erase(it);
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

static std::string irc_to_lower(std::string str)
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
