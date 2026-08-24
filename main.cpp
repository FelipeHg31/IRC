
#include "Server.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "Usage: ./ircserv <port> <password>\n";
		return 1;
	}

	int port = std::atoi(argv[1]);
	std::string password = argv[2];
	try
	{
		Server server(port, password, "ircserv");
		server.start();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return(1);
	}
	
	return 0;
}
