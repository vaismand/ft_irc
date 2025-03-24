#include "../inc/Command.hpp"
#include "../inc/Server.hpp"

/**
 * @brief Prints the Welcome Infos and the members of the Channel.
 */
void Command::printChannelWelcome(Server &server, Client &client, Channel &channel, bool isnew) {
    std::string cname = channel.getcName();
    int fd = client.getFd();
    if (isnew && channel.getcKey().empty())
        channel.broadcast(-1, ":ircserv MODE " + cname + " +nt\r\n");
    else if (isnew && !channel.getcKey().empty())
        channel.broadcast(-1, ":ircserv MODE " + cname + " +ntk " + channel.getcKey() +"\r\n");
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
    errorMap[475] = "Illegal channel name";
    errorMap[482] = "You're not channel operator";
    errorMap[502] = "Can't change mode for other users";
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
            if (channel->getJoined().size() == 1)
                server.rmChannel(*it);
        }
    }
}

void Command::updateAllModes(Server &server, int fd)
{
    Client &client = server.getClient(fd);

    client.setUserModes();
    std::vector<std::string> channels = client.getChannelList();
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++)
    {
        Channel* channel = server.getChannel(*it);
        if (channel)
        {
            channel->setModeList();
        }
    }
}

void Command::handleUserModeShow(Server &server, int fd)
{
    Client &client = server.getClient(fd);
    std::string nick = client.getNick();
    std::string userModes = client.getUserModes();
    if (userModes == "+" || userModes == "") 
        userModes = "No user modes are set";
    dvais::sendMessage(fd, ":ircserv 221 " + nick + " :" + userModes + "\r\n");
}

void Command::handleChannelModeShow(Server &server, int fd, Channel* channel)
{
    Client &requester = server.getClient(fd);
    std::string requesterNick = requester.getNick();
    std::string channelModes = channel->getChannelModes();

    dvais::sendMessage(fd, ":ircserv 324 " + requesterNick + " " 
                            + channel->getcName() + " :" + channelModes + "\r\n");

    std::ostringstream oss;
    oss << ":ircserv 329 " << requesterNick << " " << channel->getcName() << " "
        << channel->getCreationTime() << "\r\n";
    dvais::sendMessage(fd, oss.str());
}

void Command::handleUserMode(Server &server, int fd, const std::vector<std::string> &tokens)
{
    Client &client = server.getClient(fd);
    const std::string &nick = client.getNick();

    if (tokens.size() < 2) {
        sendError(fd, 461, nick, "MODE"); // ERR_NEEDMOREPARAMS
        return;
    }
    if (!tokens[1].empty() && (tokens[1][0] == '#' || tokens[1][0] == '&')) {
        handleChannelMode(server, fd, tokens);
        return;
    }
    if (tokens[1] != nick) {
        sendError(fd, 502, nick, tokens[1]); // ERR_USERSDONTMATCH
        return;
    }

    if (tokens.size() < 3) {
        handleUserModeShow(server, fd);
        return;
    }
    std::string modeStr = tokens[2];
    bool adding = true;

    for (size_t i = 0; i < modeStr.size(); i++)
    {
        if (modeStr[i] == '+') {
            adding = true;
            continue;
        } else if (modeStr[i] == '-') {
            adding = false;
            continue;
        }
        switch (modeStr[i])
        {
            case 'i':
                client.setInvisible(adding);
                break;
            default:
                sendError(fd, 501, nick, std::string(1, modeStr[i]));
                break;
        }
    }
    dvais::sendMessage(fd, ":" + nick + " MODE " + nick + " :" + modeStr + "\r\n");
}

void Command::handleChannelMode(Server &server, int fd, const std::vector<std::string> &tokens) {
    std::string clientNick = server.getClient(fd).getNick();
    std::string target = tokens[1];
    Channel* channel = server.getChannel(target);
    if (!channel) {
        sendError(fd, 403, clientNick, target); // ERR_NOSUCHCHANNEL
        return;
    }
    if (!channel->isMember(fd)) {
        sendError(fd, 442, clientNick, target); // ERR_NOTONCHANNEL
        return;
    }
    if (tokens.size() < 3) {
        handleChannelModeShow(server, fd, channel);
        return;
    }
    std::string modeStr = tokens[2];
    bool requiresOp = false;
    for (size_t i = 0; i < modeStr.size(); ++i) {
        char c = modeStr[i];
        if (c == '+' || c == '-')
            continue;
        if (c != 'b') {
            requiresOp = true;
            break;
        }
    }
    if (requiresOp && !channel->isOperator(fd))
    {
        sendError(fd, 482, clientNick, target);
        return;
    }

    bool adding = true;
    int paramIndex = 3;
    for (size_t i = 0; i < modeStr.size(); i++)
    {
        char modeChar = modeStr[i];
        if (modeChar == '+')
        {
            adding = true;
            continue;
        }
        else if (modeChar == '-')
        {
            adding = false;
            continue;
        }
        switch (modeChar) {
            case 'b':
            {
                if (adding) {
                    if (paramIndex >= (int)tokens.size()) {
                        dvais::sendMessage(fd, ":ircserv 367 " + clientNick + " " + target + " :\r\n");
                        dvais::sendMessage(fd, ":ircserv 368 " + clientNick + " " + target + " :End of Channel Ban List\r\n"); // RPL_ENDOFBANLIST
                    } else {
                        sendError(fd, 472, clientNick, "b"); 
                    }
                } else {
                    sendError(fd, 472, clientNick, "b");
                }
                break;
            }

            case 'i':
                channel->setChannelType();
                break;
            case 't':
                channel->setTopicRestricted(adding);
                break;
            case 'k': {
                if (adding) {
                    if (paramIndex >= (int)tokens.size()) {
                        sendError(fd, 461, clientNick, "MODE");
                        return;
                    }
                    std::string key = tokens[paramIndex++];
                    channel->setcKey(key);
                } else {
                    channel->setcKey("");
                }
                break;
            }
            case 'l': {
                if (adding) {
                    if (paramIndex >= (int)tokens.size()) {
                        sendError(fd, 461, clientNick, "MODE");
                        return;
                    }
                    int limit = atoi(tokens[paramIndex++].c_str());
                    channel->setUserLimit(limit);
                } else {
                    channel->setUserLimit(0);
                }
                break;
            }
            case 'o': {
                if (paramIndex >= (int)tokens.size()) {
                    sendError(fd, 461, clientNick, "MODE");
                    return;
                }
                std::string targetNick = tokens[paramIndex++];
                Client* targetClient = server.getClientByNick(targetNick);
                if (!targetClient || !channel->isMember(targetClient->getFd())) {
                    sendError(fd, 441, clientNick, targetNick);
                    continue;
                }
                if (adding) {
                    channel->addOperator(targetClient->getFd());
                } else {
                    channel->rmOperator(targetClient->getFd());
                }
                break;
            }
            case 'n': {
                channel->setNoExternalMsgs(adding);
                break;
            }
            default:
                sendError(fd, 472, clientNick, std::string(1, modeChar));
                break;
        }
    }

    std::ostringstream modeMsg;
    modeMsg << ":" << clientNick << "!" << server.getClient(fd).getUser()
            << "@" << server.getClient(fd).getIp()
            << " MODE " << target << " " << modeStr;
    for (int j = paramIndex; j < (int)tokens.size(); j++)
    {
        modeMsg << " " << tokens[j];
    }
    modeMsg << "\r\n";
    channel->broadcast(fd, modeMsg.str());
}
