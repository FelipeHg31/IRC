
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
			std::string					_password;
			std::string					_creationDate;
			std::string					_name;
			CommandHandler				_cmdHandler;
			static std::string			resolveHost(int fd);
			bool init();
			void acceptClient();
			void handlePollIn(int fd);
			void handlePollOut(int fd, std::vector<int> &toDelete);
			void execCommand(std::string cmd, Server *server, Client *client, const std::vector<std::string> &args);
		public:
			const std::string &getPass() const;
			const std::string &getName() const;
			const std::string &getCreationDate() const;
			Server(int port, const std::string &password, const std::string &name);
			~Server();

			void start();
			void sendNumericMsg(Client *client, const std::string &code, const std::string &target, const std::string &msg);
			void queueMessage(Client *client, const std::string &msg);
			void removeClient(int fd, const std::string &reason = "Client disconnected.");
			void tryRegistration(Client *client);
			
			Channel	*getChannel(const std::string &name);
			Channel	*addNewChannel(const std::string &name, Client *admin);
			Client *getClientByNick(const std::string &nick);
			std::string formatNumeric(const std::string &code, const std::string &target, const std::string &msg) const;
			std::string formatMessage(const std::string &source,const Client &speaker , const std::string& chan, const std::string& msg ) const;
			class NoValidServer : public std::exception 
			{
				private:
				const char *msg;
				public:
				NoValidServer(const char *error)  : msg(error){}
				virtual const  char * what() const throw() {return(msg);};

			};
			void broadcast(Server *server, const std::string &msg, Client *target);
};
