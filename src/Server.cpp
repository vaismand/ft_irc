/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:38:10 by dvaisman          #+#    #+#             */
/*   Updated: 2025/01/30 13:14:53 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

Server::Server()
{
}

Server::~Server()
{
}

Server::Server(const std::string &port, const std::string &pass) : _port(port), _pass(pass)
{
}

Server::Server(const Server &src) : _port(src._port), _pass(src._pass)
{
}

std::string Server::getPass() const
{
	return _pass;
}

void Server::run()
{
	_socket = createSocket();
}