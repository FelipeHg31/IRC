
#pragma once

#include <string>
#include <vector>

class Client;
class Server;

class Message
{
	public:
		Message();
		~Message();
		std::string	_cmd;
		std::vector<std::string>	_args;

		void PASS(Server *server, Client *client, std::vector<std::string> args);
};
