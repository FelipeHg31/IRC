
#pragma once

#include <string>
#include <vector>
#include <Client.hpp>

class Server;

class Channel
{
private:
	const std::string _name;
	std::vector<Client*> _clients;

	Channel();
	Channel(const Channel &rhs);
	Channel &operator=(const Channel &rhs);
	public:
	Channel(const std::string &name);
	~Channel();
	
	static bool isValidChannelName(const std::string &name);
	const std::string &getName() const;
	std::vector<Client *> &getClients();
	std::string	getMembers() const;
	Client *getClientByFd(int fd);
	void addClient(Client *client);
	void removeClient(Client *client);
	void broadcast(Server *server, const std::string &msg, Client *sender, bool toAll);
};

