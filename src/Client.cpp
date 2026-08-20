
#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _passGiven(false), _registered(false) {}

const int &Client::getFd() const { return _fd; }

const std::string &Client::getNick() const { return _nickname; }

const std::string &Client::getUser() const { return _username; }

void Client::setNick(const std::string &nick) { _nickname = nick; }

void Client::setUser(const std::string &user) { _nickname = user; }

bool Client::isPassGiven() const { return _passGiven; }

bool Client::isRegistered() const { return _registered; }

void Client::setPassGiven() { _passGiven = true; }

void Client::setRegistered() { _registered = true; }

std::set<Channel *> &Client::getChannels() { return _channels; }

std::string &Client::getInBuf() { return _inBuffer; }

std::string &Client::getOutBuf() { return _outBuffer; }

Client::~Client() {}

