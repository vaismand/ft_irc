#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/Tools.hpp"



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

std::string dvais::topicSetterTime(std::string setter, std::time_t setTime)
{
    struct tm *timeinfo = std::localtime(&setTime);  // from <ctime>
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%c", timeinfo);

    std::ostringstream oss;
    oss << setter << " " << buffer;
    return oss.str();
}

std::string dvais::extractCommand(std::string &buffer)
{
    size_t pos = buffer.find('\n');
    if (pos == std::string::npos)
        return "";
    std::string command = buffer.substr(0, pos);
    command = trim(command);
    buffer.erase(0, pos + 1);
    return command;
}

std::vector<std::string> dvais::cmdtokenizer(const std::string& command)
{
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;

    if (command.empty())
        return tokens;
    while (iss >> token) {
        if (token[0] == ':') {
            std::string rest;
            std::getline(iss, rest);
            token += rest;
            tokens.push_back(token);
            break;
        }
        tokens.push_back(token);
    }
    return tokens;
}

std::string dvais::extractTopic(std::istream &iss)
{
    std::string topic;
    std::getline(iss, topic);
    if (!topic.empty() && topic[0] == ' ')
        topic.erase(0, 1);
    if (!topic.empty() && topic[0] == ':')
        topic.erase(0, 1);
    return dvais::trim(topic);
}

std::string dvais::buildNamesList(Server &server, const Channel* channel)
{
    std::ostringstream oss;
    const std::vector<int>& members = channel->getJoined();
    for (std::vector<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
        if (channel->isOperator(*it)) {
            oss << "@" << server.getClient(*it).getNick() << " ";
        }
    }
    for (std::vector<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
        if (!channel->isOperator(*it)) {
            oss << server.getClient(*it).getNick() << " ";
        }
    }
    return oss.str();
}

std::vector<std::string> dvais::split(const std::string& cmd, char delim) {
    std::stringstream ss(cmd);
    std::string tmp;
    std::vector<std::string> cmdarg;
    while (!ss.eof()) {
        std::getline(ss, tmp, delim);
        cmdarg.push_back(tmp);
    }
    return cmdarg;
}
