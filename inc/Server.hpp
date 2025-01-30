/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:36:57 by dvaisman          #+#    #+#             */
/*   Updated: 2025/01/30 13:27:00 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/poll.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <cerrno>
#include <string>
#include "Client.hpp"
#include "Channel.hpp"

class Server
{
	private:
		const std::string _host;
		const std::string _port;
		const std::string _pass;
		int _socket;
		//Client _Client;
		//Channel _Channel;
		Server();
		Server(const Server &src);
		Server &operator=(const Server &src);
	public:
		Server(const std::string &port, const std::string &pass);
		~Server();
		
		std::string getPass() const;
		
		void run();

		int createSocket();
};