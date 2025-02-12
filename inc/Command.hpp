#pragma once

#include <string>
#include "Server.hpp"

class Server;

class Command
{
	public:
		// Constructors
		Command();
		Command(const Command &src);
		Command &operator=(const Command &src);
		~Command();

		// Methods
		void executeCommand(Server &server, int fd, const std::string &command);
	private:
		// Methods
		void commandNick(Server &server, int fd, const std::string &command);
		void commandUser(Server &server, int fd, const std::string &command);
		void commandJoin(Server &server, int fd, const std::string &command);
		void commandCap(Server &server, int fd, const std::string &command);
		void commandPing(Server &server, int fd, const std::string &command);
		void commandMode(Server &server, int fd, const std::string &command);
		void commandPass(Server &server, int fd, const std::string &command);
};
