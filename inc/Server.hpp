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
#include "Tools.hpp"

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

		// Methods
		void bindSocket();
		void addClient();
		void handleClient(int fd);
		void tryRegisterClient(int fd);

	public:
		// Constructors
		Server(const std::string &port, const std::string &pass);
		~Server();

		// Getters
		Client &getClient(int fd);
		Channel *getChannel(const std::string &name);
		std::string getClientNick(int fd) const;
		std::string getPass() const;
		const std::map<int, Client*>& getClients() const;


		// Methods
		ssize_t sendMessage(int fd, const std::string &message);
		void addChannel(const std::string &name, const std::string &pass);
		void removeClient(int fd);
		void run();
		bool isNickInUse(const std::string &nickname, int excludeFd = -1) const;

};
