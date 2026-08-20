
#pragma once

#include <string>
#include <set>

class Channel;

class Client
{
	private:
		std::string	_outBuffer;
        std::string _inBuffer;
		std::string _nickname;
		std::string _username;
		std::set<Channel *>	_channels;
	public:
		int fd;

		bool _passGiven;
		bool _registered;

		const std::string &getNick() const;
		const std::string &getUser() const;
		void setNick(const std::string &nick);
		void setUser(const std::string &user);
		std::set<Channel *> &getChannels();
		std::string	&getInBuf();
		std::string	&getOutBuf();

		Client(int fd);
		~Client();
};


