#include <Server.hpp>

void Server::queueMessage(Client *client, const std::string &msg)
{
	client->getOutBuf() += msg;
	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd == client->getFd())
		{
			_fds[i].events |= POLLOUT;
			break;
		}
	}	
}

void Server::sendNumericMsg(Client *client,
				const std::string &code, const std::string &msg)
{
	queueMessage(client, formatNumeric(code, client->getTarget(), msg));
}

std::string Server::formatNumeric(const std::string &code,
				const std::string &target, const std::string &msg) const
{
	return ":" + _name + " " + code + " " + target + " " + msg + "\r\n";
}

std::string Server::formatMessage(const std::string &source,
				const Client &speaker, const std::string &chan,
				const std::string &msg) const
{
	std::string prefix = speaker.getPrefix();
	return ":" + prefix + " " + source + " " + chan + " :" + msg + "\r\n";
}
