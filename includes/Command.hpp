
#pragma once

#include <string>
#include <vector>

class Command
{
	public:
		Command();
		~Command();
		std::string	_cmd;
		std::vector<std::string>	_args;
};
