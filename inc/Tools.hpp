#pragma once

#include <vector>
#include <stdio.h>
#include <poll.h>
#include <string>
#include <cerrno>
#include <sys/socket.h>

namespace dvais
{
	void setPollfd(int fd, short int events, std::vector<struct pollfd> &pollfds);
	ssize_t sendMessage(int fd, const std::string& message);
}