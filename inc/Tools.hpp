#pragma once

#include <vector>
#include <stdio.h>
#include <poll.h>
#include <string>
#include <cerrno>
#include <iostream>
#include <ctime>
#include <sstream>
#include <sys/socket.h>

class Server;
class Channel;

namespace dvais
{
	// Tools
	ssize_t sendMessage(int fd, const std::string& message);
	std::string topicSetterTime(std::string setter, std::time_t setTime);
	std::string extractCommand(std::string &buffer);
	std::string trim(const std::string &str);
	std::vector<std::string> cmdtokenizer(const std::string& command);
	std::string extractTopic(std::istream &iss);
	std::string buildNamesList(Server &server, const Channel* channel, int requesterFd);
	std::vector<std::string> split(const std::string& cmd, char delim);
	bool stoi(const char *str, int &nbr);
}
