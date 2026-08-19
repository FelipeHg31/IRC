/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: juan-her <juan-her@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 16:10:48 by juan-her          #+#    #+#             */
/*   Updated: 2026/08/19 22:47:43 by juan-her         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include "Client.hpp"

class Channel
{
private:
	std::string _name;
	std::vector<Client*> _clients;

public:
	Channel(const std::string &name);
	~Channel();

	std::string getName() const;
	void addClient(Client *client);
	void broadcast(const std::string &msg, Client *sender);
};

