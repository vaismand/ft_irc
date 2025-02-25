#pragma once

#include <vector>
#include <stdio.h>
#include <poll.h>
#include <string>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <sys/socket.h>

namespace dvais
{
	// Tools
	void setPollfd(int fd, short int events, std::vector<struct pollfd> &pollfds);
	ssize_t sendMessage(int fd, const std::string& message);
	std::string extractCommand(std::string &buffer);
	std::string trim(const std::string &str);
	std::vector<std::string> cmdtokenizer(const std::string& command);
	std::string extractTopic(std::istream &iss);
}