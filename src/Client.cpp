
#include "Client.hpp"

Client::Client(int fd) : fd(fd), _passGiven(false), _registered(false) {}

std::string &Client::getInBuf() { return _inBuffer; }

std::string &Client::getOutBuf() { return _outBuffer; }

Client::~Client() {}

