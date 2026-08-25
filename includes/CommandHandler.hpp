#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>

class Client;
class Server;

class CommandHandler
{
	private:
		typedef void (*Handler)(Server*, Client*, const std::vector<std::string>&);
		std::map<std::string, Handler> _commands;
		void	populateMap();
		std::set<std::string> populateRegCmds() const;
		static void announceNickChange(Server *server, Client *client, const std::string &oldNick, const std::string &newNick);
		static bool	isValidNickChar(char c, bool isFirst);

		static void PING(Server *server, Client *client, const std::vector<std::string> &args);
		static void CAP(Server *server, Client *client, const std::vector<std::string> &args);
		static void PASS(Server *server, Client *client, const std::vector<std::string> &args);
		static void ECHO(Server *server, Client *client, const std::vector<std::string> &args);
		static void NICK(Server *server, Client *client, const std::vector<std::string> &args);
		static void USER(Server *server, Client *client, const std::vector<std::string> &args);
		static void QUIT(Server *server, Client *client, const std::vector<std::string> &args);
		static void JOIN(Server *server, Client *client, const std::vector<std::string> &args);
		static void MODE(Server *server, Client *client, const std::vector<std::string> &args);
		static void INVITE(Server *server, Client *client, const std::vector<std::string> &args);
		static void TOPIC(Server *server, Client *client, const std::vector<std::string> &args);
		static void PRIVMSG(Server *server, Client *client, const std::vector<std::string> &args);
	public:
		CommandHandler();
		CommandHandler(const CommandHandler &rhs);
		CommandHandler &operator=(const CommandHandler &rhs);
		~CommandHandler();

		void	execute(std::string cmd, Server *server, Client *client, const std::vector<std::string> &args);
};
