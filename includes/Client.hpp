
#pragma once

#include <string>
#include <set>

class Channel;

class Client
{
	private:
		int _fd;
		std::string	_outBuffer;
        std::string _inBuffer;
		std::string _nickname;
		std::string _username;
		std::string	_realname;
		std::set<Channel *>	_channels;
		bool _passGiven;
		bool _registered;
	public:
		const int &getFd() const;

		const std::string &getNick() const;
		const std::string &getUser() const;
		const std::string &getRnam() const;

		void setNick(const std::string &nick);
		void setUser(const std::string &user);
		void setRnam(const std::string &name);

		bool isPassGiven() const;
		bool isRegistered() const;
		void setPassGiven();
		void setRegistered();

		std::set<Client *> getChannelPeers() const;
		std::set<Channel *> &getChannels();
		std::string	&getInBuf();
		std::string	&getOutBuf();

		Client(int fd);
		~Client();
};


