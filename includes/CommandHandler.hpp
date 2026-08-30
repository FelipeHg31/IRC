#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>

class Client;
class Server;

typedef	const std::vector<std::string> &ArgsList;

class CommandHandler
{
	public:
		CommandHandler();
		CommandHandler(const CommandHandler &rhs);
		CommandHandler &operator=(const CommandHandler &rhs);
		~CommandHandler();

		void	execute(std::string cmd, Server *server, Client *client, ArgsList args);
	private:
		typedef void (*Handler)(Server*, Client*, ArgsList);
		std::map<std::string, Handler> _commands;
		void	populateMap();
		std::set<std::string> populateRegCmds() const;
		static void announceNickChange(Server *server, Client *client, const std::string &oldNick, const std::string &newNick);
		static bool	isValidNickChar(char c, bool isFirst);

		static void PING(Server *server, Client *client, ArgsList args);
		static void CAP(Server *server, Client *client, ArgsList args);
		static void PASS(Server *server, Client *client, ArgsList args);
		static void ECHO(Server *server, Client *client, ArgsList args);
		static void NICK(Server *server, Client *client, ArgsList args);
		static void USER(Server *server, Client *client, ArgsList args);
		static void QUIT(Server *server, Client *client, ArgsList args);
		static void JOIN(Server *server, Client *client, ArgsList args);
		static void MODE(Server *server, Client *client, ArgsList args);
		static void INVITE(Server *server, Client *client, ArgsList args);
		static void TOPIC(Server *server, Client *client, ArgsList args);
		static void PRIVMSG(Server *server, Client *client, ArgsList args);
};
