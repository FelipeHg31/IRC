
#include "Client.hpp"

Client::Client(int fd) : fd(fd), registered(false) {}

Client::~Client() {}

