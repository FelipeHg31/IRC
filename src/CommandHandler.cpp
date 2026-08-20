#include <CommandHandler.hpp>
#include <Client.hpp>
#include <Channel.hpp>
#include <Server.hpp>

CommandHandler::CommandHandler()
{
	populateMap();
}

CommandHandler::CommandHandler(const CommandHandler &rhs) : _commands(rhs._commands) {}

CommandHandler &CommandHandler::operator=(const CommandHandler &rhs)
{
	if (this != &rhs)
		_commands = rhs._commands;
	return *this;
}

CommandHandler::~CommandHandler() {}

void CommandHandler::populateMap()
{
	_commands["PASS"] = &CommandHandler::PASS;
	_commands["NICK"] = &CommandHandler::NICK;
	_commands["ECHO"] = &CommandHandler::ECHO;
}

void CommandHandler::execute(std::string cmd, Server *server, Client *client, const std::vector<std::string> &args)
{
    std::map<std::string, Handler>::iterator it = _commands.find(cmd);
    if (it == _commands.end())
        return;
    it->second(server, client, args);
}
void CommandHandler::ECHO(Server *server, Client *client, const std::vector<std::string> &args)
{
	std::string reply;
	for (size_t i = 0; i < args.size(); i++)
		reply += args[i] + " ";
	reply += "\r\n";
	server->queueMessage(client, reply);
}

void CommandHandler::PASS(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (client->_passGiven)
	{
		server->queueMessage(client, server->formatError("462", client->_nickname.empty() ? "*" : client->_nickname, ":Connection already registered"));
		return;
	}
	if (args.size() != 1)
	{
		server->queueMessage(client, server->formatError("461", client->_nickname.empty() ? "*" : client->_nickname, " PASS :Syntax error"));
		return;
	}
	if (args[0] == server->getPass() && client->_nickname.empty() && client->_username.empty())
	{
		client->_passGiven = true;
	}
	else
		server->queueMessage(client, server->formatError("462", client->_nickname.empty() ? "*" : client->_nickname, " :Connection already registered"));
}

void CommandHandler::NICK(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (args.size() != 1 || args[0].empty())
	{
		server->queueMessage(client, server->formatError("461", client->_nickname.empty() ? "*" : client->_nickname, " NICK :Syntax error"));
		return;
	}
	if (args[0].size() > 9)
	{
		server->queueMessage(client, server->formatError("432", client->_nickname.empty() ? "*" : client->_nickname, args[0] + " :Nickname too long, max 9 characters"));
		return;
	}
	if (server->getClientByNick(args[0]))
	{
		server->queueMessage(client, server->formatError("433", client->_nickname.empty() ? "*" : client->_nickname, args[0] + " :Nickname already in use"));
		return;
	}
	client->_nickname = args[0];
	server->tryRegistration(client);
	server->queueMessage(client, ":prefijoprovi!sional NICK :" + args[0] + "\r\n");
	// aqui hariamos broadcast a todos los clientes de los canales en los que este client este metido
	// y aqui un tryregistrion (intentara' ver si ya tiene user y nick puestos y password y si tiene todo se conecta de forma oficial)
}

void CommandHandler::JOIN(Server *server, Client *client, const std::vector<std::string> &args)
{
	if (!client->_registered)
	{
		server->queueMessage(client, server->formatError("451", client->_nickname.empty() ? "*" : client->_nickname, " :Connection not registered"));
		return;
	}
	if (args.size() < 1)
	{
		server->queueMessage(client, server->formatError("461", client->_nickname.empty() ? "*" : client->_nickname, " JOIN :Syntax error"));
		return;
	}

	Channel *chan = server->getChannel(args[0]);
	// implementar en futuro lo de JOIN canal1,canal2 y que los canales empiezan siempre con #
	if (chan && !chan->getClientByFd(client->fd))
	{
		chan->addClient(client);
	}
	else
	{
		Channel *out = server->addNewChannel(args[0]);
		out->addClient(client);
		//new -> makeclientadmin?
	}
}
