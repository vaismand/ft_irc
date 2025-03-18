#include "Command.hpp"

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
    commandNames(server, fd, "NAMES " + cname);
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