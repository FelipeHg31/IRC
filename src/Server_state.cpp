#include <Server.hpp>

void	Server::tryRegistration(Client	*client)
{
	if (client->isRegistered())
		return;
	if (client->getNick().empty() || client->getUser().empty())
		return;
	if (!client->isPassGiven())
		return;
	client->setRegistered();

	sendNumericMsg(client, "001", ":Bienvenido al Internet Relay Network " + client->getPrefix());
	sendNumericMsg(client, "002", ":Tu host es " + _name + ", ejectuando version 1.0");
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

Client *Server::getClientByNick(const std::string &nick)
{
	ClientMap::iterator it;
	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->second->getNick() == nick)
			return it->second;
	}
	return NULL;
}
