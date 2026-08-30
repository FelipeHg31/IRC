#include <Server.hpp>
#include <Message.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <iostream>
#include <ctime>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string &password, const std::string &name): _port(port), _password(password), _name(name)
{
	if(port < 1024 || port > 49151)
		throw(NoValidServer("Bad port"));
	if(name.empty())
		throw(NoValidServer("Bad name"));
	std::time_t now = std::time(NULL);
	_creationDate = std::asctime(std::localtime(&now));
	if (!_creationDate.empty() && _creationDate[_creationDate.size() - 1] == '\n')
		_creationDate.erase(_creationDate.size() - 1);
}
Server::~Server()
{
	ClientMap::iterator clientIt;
	for (clientIt = _clients.begin(); clientIt != _clients.end(); clientIt++)
	{
		close(clientIt->first);
		delete clientIt->second;
	}
	_clients.clear();

	ChannelMap::iterator chanIt;
	for (chanIt = _channels.begin(); chanIt != _channels.end(); chanIt++)
		delete chanIt->second;
	_channels.clear();

	if (_serverSocket >= 0)
		close(_serverSocket);
}

const std::string &Server::getPass() const { return _password; }

const std::string &Server::getName() const { return _name; }

bool Server::init()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		std::cerr << "Error: bind() failed" << std::endl;
		return false;
	}

	if (listen(_serverSocket, 10) < 0)
	{
		std::cerr << "Error: listen() failed" << std::endl;
		return false;
	}

	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error: fcntl() failed" << std::endl;
		return false;
	}

	pollfd p;
	p.fd = _serverSocket;
	p.events = POLLIN;
	_fds.push_back(p);
	return true;
}

void Server::start()
{
	if (!init())
		return ;
	while (true)
	{
		int ret = poll(&_fds[0], _fds.size(), -1);
		if (ret < 0)
		{
			// decidir como manejar error de poll
			continue;
		}

		std::vector<int> toDelete;

		for (size_t i = 0; i < _fds.size(); i++)
		{
			int fd = _fds[i].fd;
			short revents = _fds[i].revents;

			if (revents == 0)
				continue;

			if (revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				toDelete.push_back(fd);
				continue;
			}
			if (fd == _serverSocket)
	   		{
				if (revents & POLLIN)
					acceptClient();
				continue;
			}
			if (revents & POLLIN)
			{
				handlePollIn(fd);
			}
			if (revents & POLLOUT)
			{
				handlePollOut(fd, toDelete);
			}
		}
		for (size_t i = 0; i < toDelete.size(); i++)
	  		removeClient(toDelete[i]);
	}
}

void Server::handlePollOut(int fd, std::vector<int>	&toDelete)
{
    Client *client = _clients[fd];
    int bytesSent = send(fd, client->getOutBuf().c_str(), client->getOutBuf().size(), 0);

    if (bytesSent < 0)
    {
        toDelete.push_back(fd);
        return;
    }

    client->getOutBuf().erase(0, bytesSent);

    if (client->getOutBuf().empty())
    {
        for (size_t j = 0; j < _fds.size(); j++)
        {
            if (_fds[j].fd == fd)
            {
                _fds[j].events &= ~POLLOUT;
                break;
            }
        }
    }
}

std::string Server::resolveHost(int fd)
{
	sockaddr_in addr;
	socklen_t len = sizeof(addr);

	if (getpeername(fd, (sockaddr *)&addr, &len) < 0)
		return "unknown";

	char *ip = inet_ntoa(addr.sin_addr);
	return ip ? std::string(ip) : "unknown";
}
void Server::acceptClient()
{
	int clientFd = accept(_serverSocket, NULL, NULL);

	if (clientFd < 0)
	{
		std::cerr << "Error: accept() failed" << std::endl;
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error: fcntl() failed" << std::endl;
		return;
	}

	pollfd p;
	p.fd = clientFd;
	p.events = POLLIN;
	p.revents = 0;
	_fds.push_back(p);

	Client *newClient = new Client(clientFd);
	newClient->setHost(resolveHost(clientFd));
	_clients[clientFd] = newClient;
	
	std::cout << "New client connected: fd=" << clientFd << " host=" << newClient->getHost() << std::endl;
}

void Server::removeClient(int fd, const std::string &reason)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;

	Client *client = it->second;

	if (client->isRegistered())
	{
		std::string prefix = client->getPrefix();
		std::string notice = ":" + prefix + " QUIT :" + reason + "\r\n";
		std::set<Client *> peers = client->getChannelPeers();
		std::set<Client *>::iterator peerIt;

		for (peerIt = peers.begin(); peerIt != peers.end(); peerIt++)
			if (*peerIt != client)
				queueMessage(*peerIt, notice);
	}

	std::set<Channel *> &channels = client->getChannels();
	std::set<Channel *>::iterator chanIt;

	for (chanIt = channels.begin(); chanIt != channels.end(); chanIt++)
		(*chanIt)->removeClient(client);

	close(fd);
	delete client;
	_clients.erase(it);

	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			break;
		}
	}

	std::cout << "Client disconnected: fd=" << fd << " host=" << client->getHost() << " reason=" << reason << std::endl;
}

void Server::sendNumericMsg(Client *client, const std::string &code, const std::string &msg)
{
	queueMessage(client, formatNumeric(code, client->getTarget(), msg));
}

void Server::queueMessage(Client *client, const std::string &msg)
{
	client->getOutBuf() += msg;
	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd == client->getFd())
		{
			_fds[i].events |= POLLOUT;
			break;
		}
	}	
}

void Server::handlePollIn(int fd)
{
	char buffer[512];
	int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes <= 0)
	{
		removeClient(fd);
		return;
	}
	buffer[bytes] = '\0';
	Client *client = _clients[fd];
	client->getInBuf() += buffer;
	size_t pos;
	while ((pos = client->getInBuf().find("\n")) != std::string::npos)
	{
		std::string cmd = client->getInBuf().substr(0, pos);
		client->getInBuf().erase(0, pos + 1);
		if (!cmd.empty() && cmd[cmd.size() - 1] == '\r')
			cmd.erase(cmd.size() - 1);
		Message	p_cmd(cmd);
		_cmdHandler.execute(p_cmd.getCmd(), this, client, p_cmd.getArgs());
	}
}

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

std::string Server::formatNumeric(const std::string &code, const std::string &target, const std::string &msg) const
{
	return ":" + _name + " " + code + " " + target + " " + msg + "\r\n";
}
std::string Server::formatMessage(const std::string &source,const Client &speaker, const std::string & chan  , const std::string& msg ) const
{
	return(( ":"+ speaker.getNick() + "!" + speaker.getUser() + "@" + speaker.getHost() + " " + source + " " + chan + " :" + msg + "\r\n"));
}

void 		Server::broadcast(Server *server, const std::string &msg, Client *target)
{
	return	server->queueMessage(target, msg);
}