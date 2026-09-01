
#include <Client.hpp>
#include <Channel.hpp>

Client::Client(int fd) : _fd(fd), _passGiven(false), _registered(false), _pendingDisconnect(false) {}

Client::~Client() {}

bool Client::operator==(const Client &rhs) { return this == &rhs; }

const int &Client::getFd() const { return _fd; }

const std::string &Client::getNick() const { return _nickname; }

const std::string &Client::getUser() const { return _username; }

const std::string &Client::getRnam() const { return _realname; }

const std::string &Client::getHost() const { return _hostname; }

const std::string &Client::getDiscReason() const { return _disconnectReason; }

std::string &Client::getInBuf() { return _inBuffer; }

std::string &Client::getOutBuf() { return _outBuffer; }

bool Client::isPassGiven() const { return _passGiven; }

bool Client::isRegistered() const { return _registered; }

bool Client::toDisconnect() const { return _pendingDisconnect; }

void Client::setNick(const std::string &nick) { _nickname = nick; }

void Client::setUser(const std::string &user) { _username = user; }

void Client::setRnam(const std::string &name) { _realname = name; }

void Client::setHost(const std::string &name) { _hostname = name; }

void Client::setToDisconnect(const std::string &reason)
{
	_pendingDisconnect = true;
	_disconnectReason = reason;
}

void Client::setPassGiven() { _passGiven = true; }

void Client::setRegistered() { _registered = true; }

std::string Client::getTarget() const
{
	return _nickname.empty() ? "*" : _nickname;
}

std::string Client::getPrefix() const
{
	return _nickname + "!" + _username + "@" + _hostname;
}

void Client::addChannel(Channel *chan) { _channels.insert(chan); }

std::set<Channel *> &Client::getChannels() { return _channels; }

std::set<Client *> Client::getChannelPeers() const
{
	std::set<Client *> peers;
	std::set<Channel *>::iterator chanIt;

	for (chanIt = _channels.begin(); chanIt != _channels.end(); chanIt++)
	{
		std::vector<Client *> &members = (*chanIt)->getClients();
		peers.insert(members.begin(), members.end());
	}
	return peers;
}
