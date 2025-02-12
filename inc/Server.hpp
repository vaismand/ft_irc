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
		// Constructors
		Server();
		Server(const Server &src);
		Server &operator=(const Server &src);

		// Attributes
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
		Channel *getChannel(const std::string &name);
		void addChannel(const std::string &name, const std::string &pass);
};