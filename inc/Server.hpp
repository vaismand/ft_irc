/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:36:57 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/12 11:21:03 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sys/socket.h>
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
		Server();
		Server(const Server &src);
		Server &operator=(const Server &src);
		const std::string _port;
		const std::string _pass;
		int _socket;
		std::vector <struct pollfd> _pollfds;
		std::map <int, Client*> _clients;
		std::map <std::string, Channel*> _channels;

		void bindSocket();
		void addClient();
		void handleClient(int fd);
		void tryRegisterClient(int fd);
		public:
		Server(const std::string &port, const std::string &pass);
		~Server();
		Client &getClient(int fd);
		std::string getClientNick(int fd) const;
		std::string getPass() const;
		ssize_t sendMessage(int fd, const std::string &message);
		
		void removeClient(int fd);
		void run();
	public:
		Channel *getChannel(const std::string &name);
		void addChannel(const std::string &name, const std::string &pass);
};