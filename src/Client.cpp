
#include "Client.hpp"

Client::Client(int fd) : fd(fd), _passGiven(false), _registered(false) {}

const std::string &Client::getNick() const { return _nickname; }

const std::string &Client::getUser() const { return _username; }

void Client::setNick(const std::string &nick) { _nickname = nick; }

void Client::setUser(const std::string &user) { _nickname = user; }

std::set<Channel *> &Client::getChannels() { return _channels; }

std::string &Client::getInBuf() { return _inBuffer; }

std::string &Client::getOutBuf() { return _outBuffer; }

Client::~Client() {}

