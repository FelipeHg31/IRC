/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: juan-her <juan-her@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 15:05:13 by juan-her          #+#    #+#             */
/*   Updated: 2026/08/19 17:26:44 by juan-her         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string &password): _port(port), _password(password)
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	bind(_serverSocket, (sockaddr*)&addr, sizeof(addr));
	listen(_serverSocket, 10);

	fcntl(_serverSocket, F_SETFL, O_NONBLOCK);

	pollfd p;
	p.fd = _serverSocket;
	p.events = POLLIN;
	_fds.push_back(p);
}

Server::~Server() {}

void Server::start()
{
	while (true)
	{
		poll(&_fds[0], _fds.size(), -1);

		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _serverSocket)
					acceptClient();
				else
					handleClient(_fds[i].fd);
			}
		}
	}
}

void Server::acceptClient()
{
	int clientFd = accept(_serverSocket, NULL, NULL);
	fcntl(clientFd, F_SETFL, O_NONBLOCK);

	pollfd p;
	p.fd = clientFd;
	p.events = POLLIN;
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

void Server::handleClient(int fd)
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
	client->buffer += buffer;
	size_t pos;
	while ((pos = client->buffer.find("\r\n")) != std::string::npos)
	{
		std::string cmd = client->buffer.substr(0, pos);
		client->buffer.erase(0, pos + 2);
		processCommand(client, cmd);
	}
}

Channel* Server::getOrCreateChannel(const std::string &name)
{
	if (_channels.find(name) == _channels.end())
		_channels[name] = new Channel(name);
	return _channels[name];
}

void Server::processCommand(Client *client, const std::string &cmd)
{
	std::istringstream iss(cmd);
	std::string command;
	iss >> command;

	if (command == "PASS")
	{
		std::string pass;
		iss >> pass;
		if (pass == _password)
			send(client->fd, "Password OK\n", 12, 0);
	}
	else if (command == "NICK")
	{
		iss >> client->nickname;
	}
	else if (command == "USER")
	{
		iss >> client->username;
		client->registered = true;
	}
	else if (command == "JOIN")
	{
		std::string channelName;
		iss >> channelName;

		Channel *channel = getOrCreateChannel(channelName);
		channel->addClient(client);

		std::string msg = client->nickname + " joined " + channelName + "\n";
		channel->broadcast(msg, client);
	}
	else if (command == "PRIVMSG")
	{
		std::string target;
		iss >> target;

		std::string msg;
		getline(iss, msg);

		if (_channels.find(target) != _channels.end())
		{
			std::string fullMsg = client->nickname + ": " + msg + "\n";
			_channels[target]->broadcast(fullMsg, client);
		}
	}
}
