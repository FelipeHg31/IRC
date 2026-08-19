/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lde-medi <lde-medi@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 15:05:13 by juan-her          #+#    #+#             */
/*   Updated: 2026/08/19 23:15:53 by lde-medi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
#include <Command.hpp>
#include <Client.hpp>
#include <Channel.hpp>
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
	while ((pos = client->buffer.find("\n")) != std::string::npos)
	{
		std::string cmd = client->buffer.substr(0, pos);
		client->buffer.erase(0, pos + 1);
		if (!cmd.empty() && cmd[cmd.size() - 1] == '\r')
			cmd.erase(cmd.size() - 1);
		processCommand(client, cmd);
	}
}

Channel* Server::getOrCreateChannel(const std::string &name)
{
	if (_channels.find(name) == _channels.end())
		_channels[name] = new Channel(name);
	return _channels[name];
}

Command	Server::parseLine(std::string line)
{
	std::istringstream iss(line);
	std::string	token;
	Command	cmd;
	while (iss >> token)
	{
		if (cmd._cmd.empty())
			cmd._cmd = token;
		else
			cmd._args.push_back(token);
	}
	return (cmd);
}

void Server::processCommand(Client *client, const std::string &cmd)
{
	Command	p_cmd = parseLine(cmd);

	if (p_cmd._cmd == "PASS")
	{
		if (p_cmd._args[0] == _password)
			send(client->fd, "Password OK\r\n", 13, 0);
		else
			send(client->fd, "Password incorrect\r\n", 21, 0);
	}
	else if (p_cmd._cmd == "NICK")
	{
		client->nickname = p_cmd._args[0];
		std::string reply = "Now talking as " + client->nickname + "\r\n";
		send(client->fd, reply.c_str(), reply.size(), 0);
	}
	else if (p_cmd._cmd == "USER")
	{
		client->username = p_cmd._args[0];
		client->registered = true;
	}
	else if (p_cmd._cmd == "JOIN")
	{
		std::string channelName;
		channelName = p_cmd._args[0];

		Channel *channel = getOrCreateChannel(channelName);
		channel->addClient(client);

		std::string msg = client->nickname + " joined " + channelName + "\n";
		channel->broadcast(msg, client);
	}
	else if (p_cmd._cmd == "PRIVMSG")
	{
		std::string target;
		target = p_cmd._args[0];

		std::string msg;
		msg = p_cmd._args[0];

		if (_channels.find(target) != _channels.end())
		{
			std::string fullMsg = client->nickname + ": " + msg + "\n";
			_channels[target]->broadcast(fullMsg, client);
		}
	}
}
