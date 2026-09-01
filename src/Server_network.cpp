#include <Server.hpp>
#include <Message.hpp>

void Server::start()
{
    init();
    while (true)
    {
        int ret = poll(&_fds[0], _fds.size(), -1);
        if (ret < 0)
            continue;

        std::vector<int> toDelete;

        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents == 0)
                continue;
            handleEvent(i, toDelete);
        }
        
        for (size_t i = 0; i < toDelete.size(); i++)
            removeClient(toDelete[i]);
    }
}

void Server::handleEvent(size_t i, std::vector<int> &toDelete)
{
    int fd = _fds[i].fd;
    short revents = _fds[i].revents;

    if (revents & (POLLHUP | POLLERR | POLLNVAL))
    {
        toDelete.push_back(fd);
        return;
    }
    
    if (fd == _serverSocket)
    {
        if (revents & POLLIN)
            acceptClient();
        return;
    }
    
    if (revents & POLLIN)
        handlePollIn(fd);
        
    if (revents & POLLOUT)
        handlePollOut(fd, toDelete);
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
	
	std::cout << "New client connected: fd=" << clientFd
					<< " host=" << newClient->getHost() << std::endl;
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
	processClientBuffer(client);
}

void Server::processClientBuffer(Client *client)
{
    size_t pos;
    while ((pos = client->getInBuf().find("\n")) != std::string::npos)
    {
        std::string cmd = client->getInBuf().substr(0, pos);
        client->getInBuf().erase(0, pos + 1);
        
        if (!cmd.empty() && cmd[cmd.size() - 1] == '\r')
            cmd.erase(cmd.size() - 1);
            
        Message p_cmd(cmd);
        _cmdHandler.execute(p_cmd.getCmd(), this, client, p_cmd.getArgs());
    }
}

void Server::handlePollOut(int fd, std::vector<int>	&toDelete)
{
    Client *client = _clients[fd];
    int bytesSent = send(fd, client->getOutBuf().c_str(),
							client->getOutBuf().size(), 0);

    if (bytesSent < 0)
    {
        toDelete.push_back(fd);
        return;
    }

    client->getOutBuf().erase(0, bytesSent);

    if (client->getOutBuf().empty())
		disablePollOut(fd);
}

void Server::disablePollOut(int fd)
{
    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events &= ~POLLOUT;
            break;
        }
    }
}

void Server::removeFromPoll(int fd)
{
	std::vector<pollfd>::iterator it;

    for (it = _fds.begin(); it != _fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _fds.erase(it);
            break;
        }
    }
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

	ChannelSet &channels = client->getChannels();
	ChannelSet::iterator chanIt;

	for (chanIt = channels.begin(); chanIt != channels.end(); chanIt++)
		(*chanIt)->removeClient(client);

	close(fd);
	delete client;
	_clients.erase(it);
	removeFromPoll(fd);
	

	std::cout << "Client disconnected: fd=" << fd << " host="
				<< client->getHost() << " reason=" << reason << std::endl;
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
