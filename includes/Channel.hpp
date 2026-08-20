
#pragma once

#include <string>
#include <vector>
#include <Client.hpp>

class Channel
{
private:
	std::string _name;
	std::vector<Client*> _clients;

public:
	Channel(const std::string &name);
	~Channel();

	const std::string &getName() const;
	std::vector<Client *> &getClients();
	Client *getClientByFd(int fd);
	void addClient(Client *client);
	void removeClient(Client *client);
	void broadcast(const std::string &msg, Client *sender);
};

