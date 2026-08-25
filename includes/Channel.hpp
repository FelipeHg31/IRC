
#pragma once

#include <string>
#include <vector>
#include <Client.hpp>

class Server;

class Channel
{
private:
	const std::string _name;
	Client *_admin;
	std::vector<Client*> _clients;
	std::vector<Client*> _invited;
	bool _inviteMode;
	Channel();
	Channel(const Channel &rhs);
	Channel &operator=(const Channel &rhs);
	public:
	Channel(const std::string &name, Client *admin);
	~Channel();
	void putUpInviteMode();
	void putDownInviteMode();
	const Client &getAdmin();
	static bool isValidChannelName(const std::string &name);
	const std::string &getName() const;
	std::vector<Client *> &getClients();
	std::string	getMembers() const;
	Client *getClientByFd(int fd);
	Client *getInvitedbyFd(int fd);
	void  Inviteclient(Client* other);
	void  RemoveInvite(Client* other);
	bool IsInvited(Client *other);
	void addClient(Client *client);
	void removeClient(Client *client);
	void broadcast(Server *server, const std::string &msg, Client *sender, bool toAll);
	bool isAdmin( Client& other);
	bool inviteMode();
};

