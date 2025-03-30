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
#include <csignal>
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Tools.hpp"
#include "../inc/Bot.hpp"

#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

extern volatile bool g_running;

enum Limits {
	MAX_DEFAULT_LEN = 16,
	MAX_CHAN_LEN = 20,
	MAX_TOPIC_LEN = 200,
};

class Server
{
	public:
		// Constructors
		Server(const std::string &port, const std::string &pass);
		~Server();

		// Getters
		Client &getClient(int fd) const;
		Channel *getChannel(const std::string &name);
		std::string getClientNick(int fd) const;
		std::string getPass() const;
		const std::map<int, Client*>& getClients() const;
		Client *getClientByNick(const std::string &nickname);
		std::size_t getChannelLimit() const;
		Bot &getBot() const;

		// Methods
		void addChannel(const int &fd, const std::string &name, const std::string &pass);
		void rmChannel(const std::string &name);
		void rmClient(int fd);
		void run();
		void createBot();
		void broadcastAll(int fd, const std::string &msg);
		void checkEmptyChannels();
		bool isNickInUse(const std::string &nickname, int excludeFd = -1) const;
		bool isValidNick(const std::string &nickname);
		bool shareChannel(int fd1, int fd2) const;
	
	private:
		// Constructors
		Server();
		Server (const Server &obj);
		Server& operator= (const Server &rhs);
		
		// Methods
		void bindSocket();
		void addClient();
		private:
		void handleClient(int fd);
		void tryRegisterClient(int fd);
		void checkIdleClients();
		void setPollfd(int fd, short int events, std::vector<struct pollfd> &pollfds);
		void welcomeToServerMessage(int fd, const std::string &nick);

		// Attributes
		const std::string 					_port;
		const std::string 					_pass;
		int 								_socket;
		std::size_t 						_channelLimit;
		std::vector <struct pollfd> 		_pollfds;
		std::map <int, Client*> 			_clients;
		std::map <std::string, Channel*> 	_channels;
		Bot* 								bot_;
		Command 							_cmd;
	};
