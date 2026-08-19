/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: juan-her <juan-her@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 16:15:43 by juan-her          #+#    #+#             */
/*   Updated: 2026/08/19 18:45:57 by juan-her         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class Client
{
public:
    int fd;
    std::string nickname;
    std::string username;
    std::string buffer;
    bool registered;

    Client(int fd);
    ~Client();
};


