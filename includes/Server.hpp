
#pragma once

#include <string>
#include <vector>
#include <map>
#include <CommandHandler.hpp>
#include <poll.h>

class Message;
class Client;
class Channel;

class Server
{
		private:
			int							_serverSocket;
			int							_port;
			std::vector<struct pollfd>	_fds; 
			std::map<int, Client *>		_clients;
			std::map<std::string, Channel *> _channels;
			
		public:
			CommandHandler				_cmdHandler;
			std::string					_password;
			Server(int port, const std::string &password);
			~Server();
			bool init();
			void start();
			void acceptClient();
			void handlePollIn(int fd);
			void handlePollOut(int fd, std::vector<int> &toDelete);
			void removeClient(int fd);
			void queueMessage(Client *client, const std::string &msg);
			void processMessage(Client *client, const std::string &cmd);
			Channel	*getChannel(const std::string &name);
			Channel	*addNewChannel(const std::string &name);
			Client *getClientByNick(const std::string &nick);
			std::string formatError(std::string code, const std::string &target, const std::string &msg) const;
			class NoValidServer : public std::exception 
			{
				private:
				const char *msg;
				public:
				NoValidServer(const char *error)  : msg(error){}
				virtual const  char * what() const throw() {return(msg);};

			};
};

