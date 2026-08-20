#include <Server.hpp>
#include <Message.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string &password): _port(port), _password(password)
{}

Server::~Server() {}

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

void Server::acceptClient()
{
	int clientFd = accept(_serverSocket, NULL, NULL);

	if (clientFd < 0)
	{
		std::cerr << "Error: accept() failed" << std::endl;
		return;
	}
	fcntl(clientFd, F_SETFL, O_NONBLOCK);

	pollfd p;
	p.fd = clientFd;
	p.events = POLLIN;
	p.revents = 0;
	_fds.push_back(p);

	_clients[clientFd] = new Client(clientFd);

	std::cout << "New client connected\n";
}

void Server::removeClient(int fd)
{
	close(fd);
	delete _clients[fd];
	_clients.erase(fd);

	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			break;
		}
	}
}

void Server::queueMessage(Client *client, const std::string &msg)
{
	client->getOutBuf() += msg;
	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd == client->fd)
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
		processMessage(client, cmd);
	}
}

Channel* Server::getOrCreateChannel(const std::string &name)
{
	if (_channels.find(name) == _channels.end())
		_channels[name] = new Channel(name);
	return _channels[name];
}

void Server::processMessage(Client *client, const std::string &cmd)
{
	Message	p_cmd(cmd);

	_cmdHandler.execute(p_cmd.getCmd(), this, client, p_cmd.getArgs());
}
