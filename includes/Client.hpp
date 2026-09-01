
#pragma once

#include <string>
#include <set>

class Channel;

typedef std::set<Channel *> ChannelSet;

class Client
{
	private:
		int _fd;
		std::string	_outBuffer;
        std::string _inBuffer;
		std::string _nickname;
		std::string _username;
		std::string	_realname;
		std::string	_hostname;
		ChannelSet	_channels;
		bool _passGiven;
		bool _registered;

		Client();
		Client(const Client &rhs);
		Client &operator=(const Client &rhs);
	public:
		Client(int fd);
		~Client();
		
		bool	operator==(const Client& rhs);
		
		const int &getFd() const;

		const std::string &getNick() const;
		const std::string &getUser() const;
		const std::string &getRnam() const;
		const std::string &getHost() const;

		std::string	&getInBuf();
		std::string	&getOutBuf();
		
		bool isPassGiven() const;
		bool isRegistered() const;

		void setNick(const std::string &nick);
		void setUser(const std::string &user);
		void setRnam(const std::string &name);
		void setHost(const std::string &host);
		
		void setPassGiven();
		void setRegistered();
		

		
		std::string getTarget() const;
		std::string	getPrefix() const;
		
		void addChannel(Channel *chan);
		
		ChannelSet &getChannels();
		std::set<Client *> getChannelPeers() const;
};


