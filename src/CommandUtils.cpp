#include "../inc/Command.hpp"
#include "../inc/Server.hpp"

/**
 * @brief Prints the Welcome Infos and the members of the Channel.
 */
void Command::printChannelWelcome(Server &server, Client &client, Channel &channel, bool isnew) {
    std::string cname = channel.getcName();
    int fd = client.getFd();
    if (isnew)
        channel.broadcast(-1, ":ircserv MODE " + cname + " +nt\r\n");
    if (!channel.getcTopic().empty()) {
        time_t topicTime = channel.getTopicSetTime();
        std::ostringstream oss;
        oss << topicTime;
        std::string timeStr = oss.str();
        dvais::sendMessage(fd, ":ircserv 332 " + client.getNick() + " " + cname + " :" + channel.getcTopic() + "\r\n");
        dvais::sendMessage(fd, ":ircserv 333 " + client.getNick() + " " + cname + " " 
                                                                + channel.getTopicSetter() + " " + timeStr + "\r\n");
    }
    executeCommand(server, fd, "NAMES " + cname);
}

void Command::initErrorMap()
{
    errorMap[331] = "No topic is set";
    errorMap[401] = "No such nick/channel";
    errorMap[403] = "No such channel";
    errorMap[404] = "Cannot send to channel";
    errorMap[405] = "You have joined too many channels";
    errorMap[411] = "No recipient given";
    errorMap[412] = "No text to send";
    errorMap[421] = "Unknown command";
    errorMap[431] = "No nickname given";
    errorMap[432] = "Erroneous Nickname";
    errorMap[433] = "Nickname is already in use";
    errorMap[442] = "You're not on that channel";
    errorMap[451] = "You have not registered";
    errorMap[461] = "Not enough parameters";
    errorMap[462] = "You may not reregister";
    errorMap[464] = "Password incorrect";
    errorMap[471] = "Channel is full";
    errorMap[472] = "is unknown mode char to me";
    errorMap[473] = "Cannot join channel (invite only)";
    errorMap[475] = "Cannot join channel (+k)";
    errorMap[482] = "You're not channel operator";
}

void Command::sendError(int fd, int errorCode, const std::string &nick, const std::string &command)
{
    std::ostringstream oss;
    oss << ":ircserv " << errorCode << " " << nick << " ";
    if (!command.empty()) {
        oss << command << " ";
    }
    oss << ":" << errorMap[errorCode] << "\r\n";
    dvais::sendMessage(fd, oss.str());
}

/**
 * @brief Parts Client from all specified Channels.
 * Iterates over the provided list of channel names and sends a PART message for each.
 */
void Command::partClientAll(Server &server, Client &client, std::vector<std::string> list, std::string reason) {
    std::string user = client.getUser();
    std::string host = client.getIp();
    std::string nick = client.getNick();
    std::string prefix = ":" + nick + "!" + user + "@" + host;

    for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); it++) {
        std::string msg = prefix + " PART " + *it + " " + reason + "\r\n";
        Channel* channel = server.getChannel(*it);
        if (channel) {
            channel->broadcast(client.getFd(), msg);
            dvais::sendMessage(client.getFd(), msg);
            channel->rmClient(client.getFd());
            client.rmChannelInList(channel->getcName());
            if (channel->getJoined().empty())
                server.rmChannel(*it);
        }
    }
}
