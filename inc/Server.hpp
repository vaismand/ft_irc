/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:36:57 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/09 19:15:37 by dvaisman         ###   ########.fr       */
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
#include <map>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

class Command;

class Server
{
	private:
		const std::string _port;
		const std::string _pass;
		int _socket;
		std::vector <struct pollfd> _pollfds;
		std::map <int, Client*> _clients;
		std::map <int, Channel*> _channel;
		Server();
		Server(const Server &src);
		Server &operator=(const Server &src);

		void bindSocket();
		void addClient();
		void removeClient(int fd);
		void handleClient(int fd);
		void checkAuth(int fd, const char *buffer);
		void sendTimeStamps(int fd);
	public:
		Server(const std::string &port, const std::string &pass);
		~Server();
		Client &getClient(int fd);
		void addClientToChannel(int fd, const std::string &channel);
		void broadcastToChannel(const std::string &channel, const std::string &msg);
		std::string getClientNick(int fd) const;
		std::string getPass() const;
		
		void run();
};