
#pragma once

#include <string>
#include <vector>
#include "Client.hpp"

class Channel
{
private:
	std::string _name;
	std::vector<Client*> _clients;

public:
	Channel(const std::string &name);
	~Channel();

	std::string getName() const;
	void addClient(Client *client);
	void broadcast(const std::string &msg, Client *sender);
};

