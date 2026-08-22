
#pragma once

#include <string>
#include <vector>
#include <Client.hpp>

class Server;

class Channel
{
private:
	std::string _name;
	std::vector<Client*> _clients;
	Channel(const Channel &rhs);
	Channel &operator=(const Channel &rhs);
public:
	Channel(const std::string &name);
	~Channel();

	const std::string &getName() const;
	std::vector<Client *> &getClients();
	Client *getClientByFd(int fd);
	void addClient(Client *client);
	void removeClient(Client *client);
	void broadcast(Server *server, const std::string &msg, Client *sender, bool toAll);
};

