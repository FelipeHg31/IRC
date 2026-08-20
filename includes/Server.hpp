
#pragma once

#include <string>
#include <vector>
#include <map>
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
			std::map<int, Client*>		_clients;
			std::map<std::string, Channel*> _channels;
			
		public:
			std::string					_password;
			Server(int port, const std::string &password);
			~Server();
			bool init();
			void start();
			void acceptClient();
			void handleClient(int fd);
			void removeClient(int fd);
			void processMessage(Client *client, const std::string &cmd);
			Channel* getOrCreateChannel(const std::string &name);
			Message	parseLine(std::string line);
};

