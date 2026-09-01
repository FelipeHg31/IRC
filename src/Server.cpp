#include <Server.hpp>

Server::Server(int port, const std::string &password, const std::string &name): _port(port), _name(name), _password(password)
{
	if(port < 1024 || port > 49151)
		throw(NoValidServer("Bad port"));
	if(name.empty())
		throw(NoValidServer("Bad name"));
	std::time_t now = std::time(NULL);
	_creationDate = std::asctime(std::localtime(&now));
	if (!_creationDate.empty() && _creationDate[_creationDate.size() - 1] == '\n')
		_creationDate.erase(_creationDate.size() - 1);
}
Server::~Server()
{
	ClientMap::iterator clientIt;
	for (clientIt = _clients.begin(); clientIt != _clients.end(); clientIt++)
	{
		close(clientIt->first);
		delete clientIt->second;
	}
	_clients.clear();

	ChannelMap::iterator chanIt;
	for (chanIt = _channels.begin(); chanIt != _channels.end(); chanIt++)
		delete chanIt->second;
	_channels.clear();

	if (_serverSocket >= 0)
		close(_serverSocket);
}

void Server::init()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Error: bind() failed");
	if (listen(_serverSocket, 10) < 0)
		throw std::runtime_error("Error: listen() failed");
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Error: fcntl() failed");

	pollfd p;
	p.fd = _serverSocket;
	p.events = POLLIN;
	_fds.push_back(p);
}

const std::string &Server::getPass() const { return _password; }

const std::string &Server::getName() const { return _name; }