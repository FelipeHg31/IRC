#pragma once

#include <Client.hpp>
#include <Channel.hpp>
#include <CommandHandler.hpp>
#include <iostream>
#include <ctime>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <map>
#include <poll.h>

class Message;
class Client;
class Channel;

typedef std::map<int, Client *> ClientMap;
typedef std::map<std::string, Channel *> ChannelMap;
typedef std::vector<struct pollfd> FdVector;

class Server
{
	public:
		Server(int port, const std::string &password, const std::string &name);
		~Server();
		const std::string &getPass() const;
		const std::string &getName() const;
		const std::string &getCreationDate() const;

		void start();
		
		void tryRegistration(Client *client);		
		const ChannelMap  &getChannels() const;
		Channel	*getChannel(const std::string &name);
		Channel	*addNewChannel(const std::string &name, Client *admin);
		void	removeChannel(const std::string &name);
		Client *getClientByNick(const std::string &nick);

		void removeClient(int fd,
				const std::string &reason = "Client disconnected.");
				
		void queueMessage(Client *client, const std::string &msg);
		void sendNumericMsg(Client *client,
				const std::string &code, const std::string &msg);

		std::string formatNumeric(const std::string &code,
				const std::string &target, const std::string &msg) const;
		std::string formatMessage(const std::string &source,
				const Client &speaker , const std::string& chan, 
				const std::string& msg ) const;

		class NoValidServer : public std::exception 
		{
			private:
				const char *msg;
			public:
				NoValidServer(const char *error)  : msg(error){}
				virtual const  char * what() const throw() {return(msg);};

		};
	private:
		int			_port;
		int			_serverSocket;
		std::string	_name;
		FdVector	_fds; 
		std::string	_password;
		std::string	_creationDate;
		ClientMap	_clients;
		ChannelMap	_channels;

		CommandHandler	_cmdHandler;

		void init();

		void acceptClient();
		void handlePollIn(int fd);
		void handlePollOut(int fd);
		void disablePollOut(int fd);
		void removeFromPoll(int fd);
		void removePendingClients();
		void processClientBuffer(Client *client);
		void handleEvent(size_t i);
		std::string resolveHost(int fd);
};
