#include "../inc/Tools.hpp"

void dvais::setPollfd(int fd, short int events, std::vector<struct pollfd> &pollfds)
{
	struct pollfd pollfd;
	pollfd.fd = fd;
	pollfd.events = events;
	pollfd.revents = 0;
	pollfds.push_back(pollfd);
}

ssize_t dvais::sendMessage(int fd, const std::string& message)
{
    size_t totalSent = 0;
    size_t messageLen = message.size();
    const char* msg = message.c_str();

    while (totalSent < messageLen)
    {
        ssize_t sent = send(fd, msg + totalSent, messageLen - totalSent, 0);
        if (sent < 0)
        {
            if (errno == EINTR)
                continue;
            perror("send");
            return -1;
        }
        totalSent += sent;
    }
    return totalSent;
}

std::string dvais::trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

std::string dvais::extractCommand(std::string &buffer)
{
    size_t pos = buffer.find('\n');
    if (pos == std::string::npos)
        return "";
    std::string command = buffer.substr(0, pos);
    // Fully trim the command: remove leading and trailing whitespace.
    command = trim(command);
    buffer.erase(0, pos + 1);
    return command;
}
