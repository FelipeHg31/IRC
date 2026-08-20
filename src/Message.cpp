
#include <Message.hpp>
#include <Client.hpp>
#include <Server.hpp>
#include <sys/socket.h>

Message::Message() {}

Message::~Message() {}

void Message::PASS(Server *server, Client *client, std::vector<std::string> args)
{
	if (client->registered)
		send(client->fd, "already registered error\r\n", 28, 0);
	else
	{
		if (args[0] == server->_password)
			send(client->fd, "Password OK\r\n", 13, 0);
		else
			send(client->fd, "Password incorrect\r\n", 21, 0);		
	}
}