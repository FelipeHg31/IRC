
#pragma once

#include <string>
#include <vector>
#include <Client.hpp>

class Server;

class Channel
{
private:
	const std::string _name;
	std::string _topic;
	std::set<Client *>	_operators;
	std::vector<Client*> _clients;
	std::vector<Client*> _invited;
	bool _inviteMode;
	Channel();
	Channel(const Channel &rhs);
	Channel &operator=(const Channel &rhs);
public:
	Channel(const std::string &name, Client *admin);
	~Channel();
	void setTopic(const std::string& topic);
	std::set<Client *> &getOperators();
	const std::string &getName() const;
	const std::string &getTopic() const;
	std::vector<Client *> &getClients();
	std::string	getMembers() const;
	Client *getClientByFd(int fd);
	Client *getInvitedbyFd(int fd);
	void  Inviteclient(Client* other);
	void  RemoveInvite(Client* other);
	void addClient(Client *client);
	void removeClient(Client *client);
	void putUpInviteMode();
	void putDownInviteMode();
	void broadcast(Server *server, const std::string &msg, Client *sender, bool toAll);
	static bool isValidChannelName(const std::string &name);
	bool isAdmin( Client& other);
	bool inviteMode();
	bool IsInvited(Client *other);
};

