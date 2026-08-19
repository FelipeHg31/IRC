/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lde-medi <lde-medi@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 21:27:02 by lde-medi          #+#    #+#             */
/*   Updated: 2026/08/19 21:29:52 by lde-medi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
