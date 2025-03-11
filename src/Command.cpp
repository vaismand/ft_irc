#include "../inc/Command.hpp"

Command::Command()
{
    initErrorMap();
}

Command::Command(const Command &src)
{
    (void)src;
}

Command &Command::operator=(const Command &src)
{
    (void)src;
    return *this;
}

Command::~Command() {}

void Command::initErrorMap()
{
    errorMap[331] = "No topic is set";
    errorMap[401] = "No such nick/channel";
    errorMap[403] = "No such channel";
    errorMap[404] = "Cannot send to channel";
    errorMap[405] = "You're already on that channel";
    errorMap[411] = "No recipient given";
    errorMap[412] = "No text to send";
    errorMap[421] = "Unknown command";
    errorMap[431] = "No nickname given";
    errorMap[432] = "Erroneous Nickname";
    errorMap[433] = "Nickname is already in use";
    errorMap[442] = "Not on channel";
    errorMap[451] = "You have not registered";
    errorMap[461] = "Not enough parameters";
    errorMap[462] = "You may not reregister";
    errorMap[464] = "Password incorrect";
    errorMap[471] = "Channel is full";
    errorMap[472] = "is unknown mode char to me";
    errorMap[473] = "Cannot join channel (invite only)";
    errorMap[475] = "Cannot join channel (invite only)";
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

void Command::commandCap(int fd, const std::string &command)
{
    if (command.find("CAP LS") == 0) {
        dvais::sendMessage(fd, "CAP * LS :multi-prefix away-notify\r\n");
        return;
    }
    if (command.find("CAP REQ") == 0) {
        dvais::sendMessage(fd, "CAP * ACK :multi-prefix away-notify\r\n");
        return;
    }
    if (command.find("CAP END") == 0) {
        return;
    }
    sendError(fd, 421, "", "CAP");
}
/**
 * @brief Handles the NICK command in the IRC server.
 * This command sets and updates the nickname of the client. It needs a valid nickname as parameter.
 * Nickname change is broadcasted to all.
 * 
 * @throws ERR_NONICKNAMEGIVEN (431) - if no nickname is provided.
 * @throws ERR_ERRONEUSNICKNAME (432) - if provided nickname has invalid format.
 * @throws ERR_NICKNAMEINUSE (433) - if provided nickname is already taken by another client.
 */
void Command::commandNick(Server &server, int fd, const std::string &command) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    Client &client = server.getClient(fd);
    const std::string &currentNick = client.getNick();
    if (tokens.size() < 2) {
        sendError(fd, 431, currentNick, ""); // ERR_NONICKNAMEGIVEN 
        return;
    }
    std::string nickname = tokens[1]; 
    if (server.isNickInUse(nickname, fd)) {
        sendError(fd, 433, currentNick, nickname); // ERR_NICKNAMEINUSE
        return;
    }
    if (nickname.empty() || !server.isValidNick(nickname)) {
        sendError(fd, 432, currentNick, nickname); // ERR_ERRONEUSNICKNAME
        return;
    }
    std::string user = client.getUser();
    std::string host = client.getIp();
    std::string msg = ":" + currentNick + "!" + user + "@" + host + " NICK :" + nickname + "\r\n";
    client.setNick(nickname);
    dvais::sendMessage(fd, msg);
    server.broadcastAll(fd, msg);
}

/**
 * @brief Handles the USER command in the IRC server.
 * This command sets the username for the client. It needs username as parameter and can
 * be set only once.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no username is provided.
 * @throws ERR_ALREADYREGISTERED (462) - if client is already registered.
 */
void Command::commandUser(Server &server, int fd, const std::string &command) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    Client &client = server.getClient(fd);
    const std::string &nick = client.getNick();
    if (tokens.size() < 2 || tokens[1].empty()) {
        sendError(fd, 461, nick, "USER"); // ERR_NEEDMOREPARAMS
        return;
    }
    if (client.getStatus() == REGISTERED) {
        sendError(fd, 462, nick, "USER"); // ERR_ALREADYREGISTERED
        return;
    }
    std::string username = tokens[1];
    client.setUser(username);
    dvais::sendMessage(fd, "Username set to " + username + "\r\n");
}

/**
 * @brief Handles the JOIN command in the IRC server.
 * This command allows a client to join existing channels or create new channels.
 * Processes comma-separated lists of channels and keys. For each channel:
 * - Validates the channel name.
 * - Checks that the client is not exceeding the channel limit.
 * - Creates the channel if it doesn't exist (using the provided key if any).
 * - Verifies that the client can join (key, invite, user limit, etc.).
 * - Adds the client to the channel and broadcasts the JOIN message.
 * - Finally, sends topic and names replies.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no channel is provided.
 * @throws ERR_NOSUCHCHANNEL (403) - if a specified channel is invalid.
 * @throws ERR_TOOMANYCHANNELS (405) - if the client has exceeded the maximum number of channels.
 * @throws ERR_BADCHANNELKEY (475) - if the provided key does not match the channel's key.
 * @throws ERR_CHANNELISFULL (471) - if the channel has reached its user limit.
 * @throws ERR_INVITEONLYCHAN (473) - if the channel is invite-only and the client is not invited.
 */
void Command::commandJoin(Server &server, int fd, const std::string &command) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    Client &client = server.getClient(fd);
    const std::string &nick = client.getNick();
    if (tokens.size() < 2 || tokens[1].empty()) {
        sendError(fd, 461, nick, "JOIN"); // ERR_NEEDMOREPARAMS
        return;
    }
    std::vector<std::string> channels = dvais::split(tokens[1], ',');
    std::vector<std::string> keys;
    if (tokens.size() >= 3 && !tokens[2].empty()) {
        keys = dvais::split(tokens[2], ',');
    }
    if (!channels.empty() && channels[0] == "0") {
        partClientAll(server, client);
        channels.erase(channels.begin());
        if (!keys.empty())
            keys.erase(keys.begin());
    }
    for (int i = 0; i < channels.size(); i++) {
        if (channels[i].find("#") != 0) {
            sendError(fd, 403, nick, channels[i]); // ERR_NOSUCHCHANNEL
            continue;
        }
        if (channels.size() > server.getChannelLimit()) {
            sendError(fd, 405, nick, channels[i]); // ERR_TOOMANYCHANNELS
            break;
        }
        Channel* ChannelToJoin = server.getChannel(channels[i]);
        bool newChannel = false;
        if (ChannelToJoin == NULL) {
            std::string key = (i < keys.size()) ? keys[i] : "";
            server.addChannel(fd, channels[i], key);
            ChannelToJoin = server.getChannel(channels[i]);
            newChannel = true;
        } else {
            if (ChannelToJoin->isMember(fd))
                continue;
            const size_t userLimit = ChannelToJoin->getUserLimit();
            if (userLimit > 0 && ChannelToJoin->getJoined().size() >= userLimit) {
                sendError(fd, 471, nick, channels[i]); // ERR_CHANNELISFULL
                continue;
            }
            if (ChannelToJoin->getChannelType() && !ChannelToJoin->isInvited(fd)) {
                sendError(fd, 473, nick, channels[i]); // ERR_INVITEONLYCHAN
                continue;
            }
            if (!ChannelToJoin->getcKey().empty() && (keys.empty() || keys[i] != ChannelToJoin->getcKey())) {
                sendError(fd, 475, nick, channels[i]); // ERR_BADCHANNELKEY 
                continue;
            }
        }
        ChannelToJoin->addClient(fd);
        std::string user = client.getUser();
        std::string host = client.getIp();
        client.setChannelList(ChannelToJoin->getcName());
        std::string prefix = ":" + nick + "!" + user + "@" + host;
        ChannelToJoin->broadcast(-1, prefix + " " + tokens[0] + " " + channels[i] + "\r\n");
        printChannelWelcome(server, client, *ChannelToJoin, newChannel);
        if (ChannelToJoin->isInvited(fd))
        ChannelToJoin->rmInvited(fd);
    }
}

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
 * @brief Parts Client of all Channels.
 */
void Command::partClientAll(Server &server, Client &client) {
    std::vector<std::string> list = client.getChannelList();
    std::string user = client.getUser();
    std::string host = client.getIp();
    std::string nick = client.getNick();
    std::string prefix = ":" + nick + "!" + user + "@" + host;

    for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); it++) {
        std::string msg = prefix + " PART " + *it + "" + "\r\n";
        server.getChannel(*it)->broadcast(client.getFd(), msg);
        dvais::sendMessage(client.getFd(), msg);
        if (server.getChannel(*it)->getJoined().empty())
            server.rmChannel(*it);
    }
}

/**
 * @brief Handles the PART command in the IRC server.
 * 
 *
 */
void Command::commandPart(Server &server, int fd, const std::string &command) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    Client &client = server.getClient(fd);
    const std::string &nick = client.getNick();
    if (tokens.size() < 2) {
        sendError(fd, 461, nick, "PART"); // ERR_NEEDMOREPARAMS 
        return;
    }
    std::string reason = (tokens.size() >= 3 && !tokens[2].empty() && tokens[2][0] == ':') ? tokens[2] : "";

    std::string channelName = tokens[1];
    Channel* channelToPart = server.getChannel(channelName);
    if (!channelToPart) {
        sendError(fd, 403, nick, channelName); // ERR_NOSUCHCHANNEL
        return;
    }
    if (!channelToPart->isMember(fd)) {
        sendError(fd, 442, nick, channelName); // ERR_NOTONCHANNEL 
        return;
    }
    std::string user = client.getUser();
    std::string host = client.getIp();
    std::string prefix = ":" + nick + "!" + user + "@" + host;
    std::string msg = prefix + " PART " + channelName + " " + reason + "\r\n";
    channelToPart->broadcast(fd, msg);
    dvais::sendMessage(fd, msg);
    channelToPart->rmClient(fd);
    if (channelToPart->getJoined().empty())
        server.rmChannel(channelName);
}

void Command::commandWhois(Server &server, int fd, const std::string &command) {
    std::istringstream iss(command);
    std::string cmd, targetNick;
    iss >> cmd >> targetNick;
    targetNick = dvais::trim(targetNick);

    if (targetNick.empty())
    {
        sendError(fd, 461, server.getClient(fd).getNick(), "WHOIS");
        return;
    }
    Client* target = server.getClientByNick(targetNick);
    if (!target)
    {
        sendError(fd, 401, server.getClient(fd).getNick(), targetNick);
        return;
    }
    std::string requesterNick = server.getClient(fd).getNick();
    std::ostringstream oss;
    oss << ":ircserv 311 " << requesterNick << " " << target->getNick()
        << " :[~" << target->getNick() << "@" << target->getIp() << "]\r\n";
    dvais::sendMessage(fd, oss.str());

    oss.str("");
    oss << ":ircserv 312 " << requesterNick << " " << target->getNick()
        << " ircserv :Welcome to ircserv\r\n";
    dvais::sendMessage(fd, oss.str());

    oss.str("");
    oss << ":ircserv 318 " << requesterNick << " " << target->getNick() << " :End of WHOIS list\r\n";
    dvais::sendMessage(fd, oss.str());
}

void Command::commandWho(Server &server, int fd, const std::string &command) {
    std::istringstream iss(command);
    std::string cmd, targetNick;
    iss >> cmd >> targetNick;
    targetNick = dvais::trim(targetNick);

    if (targetNick.empty())
    {
        sendError(fd, 461, server.getClient(fd).getNick(), "WHO");
        return;
    }
    Channel* channel = server.getChannel(targetNick);
    if (!channel)
    {
        sendError(fd, 403, server.getClient(fd).getNick(), targetNick);
        return;
    }
    std::string requesterNick = server.getClient(fd).getNick();
    std::ostringstream oss;
    oss << ":ircserv 352 " << requesterNick << " " << targetNick << " ircserv ircserv " << channel->getcName() << " H :0 ircserv\r\n";
    dvais::sendMessage(fd, oss.str());

    oss.str("");
    oss << ":ircserv 315 " << requesterNick << " " << targetNick << " :End of WHO list\r\n";
    dvais::sendMessage(fd, oss.str());
}

void Command::commandMode(Server &server, int fd, const std::string &command)
{
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    std::string clientNick = server.getClient(fd).getNick();

    if (tokens.size() < 2) {
        sendError(fd, 461, clientNick, "MODE");
        return;
    }
    std::string target = tokens[1];
    if (target == clientNick) {
        dvais::sendMessage(fd, ":ircserv 221 " + clientNick + " :\r\n");
        return;
    }
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = server.getChannel(target);
        if (!channel) {
            sendError(fd, 403, clientNick, target);
            return;
        }
        if (!channel->isMember(fd)) {
            sendError(fd, 442, clientNick, target);
            return;
        }
        if (tokens.size() < 3) {
            dvais::sendMessage(fd, ":ircserv 324 " + clientNick + " " + target + " :+it\r\n");
            dvais::sendMessage(fd, ":ircserv 329 " + clientNick + " " + target + " 1678901234\r\n");
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
        if (requiresOp && !channel->isOperator(fd)) {
            sendError(fd, 482, clientNick, target);
            return;
        }
        int paramIndex = 3;
        bool adding = true;
        for (size_t i = 0; i < modeStr.length(); i++)
        {
            char modeChar = modeStr[i];
            if (modeChar == '+') {
                adding = true;
                continue;
            } else if (modeChar == '-') {
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
        for (int j = 3; j < (int)tokens.size(); j++) {
            modeMsg << " " << tokens[j];
        }
        modeMsg << "\r\n";
        channel->broadcast(fd, modeMsg.str());
        return;
    }
    sendError(fd, 421, clientNick, "MODE");
}

void Command::commandPing(int fd, const std::string &command) {
    std::string servername = command.substr(5);
    dvais::sendMessage(fd, "PONG " + servername + "\r\n");
}

void Command::commandPass(Server &server, int fd, const std::string &command) {
    std::string providedPass = command.substr(5);
    if (providedPass != server.getPass()) {
        sendError(fd, 464, "", "");
        server.rmClient(fd);
    } else {
        server.getClient(fd).setPassAccepted(true);
    }
}

void Command::commandMsg(Server &server, int fd, const std::string &command, bool sendErs) {
    std::vector<std::string> cmd = dvais::cmdtokenizer(command);
    if (cmd.size() < 3) {
        if (sendErs)
            sendError(fd, 411, server.getClient(fd).getNick(), cmd[0]);
        return;
    }
    if (cmd[2].empty() || cmd[2][0] != ':') {
        if (sendErs)
            sendError(fd, 412, server.getClient(fd).getNick(), "PRIVMSG");
        return;
    }
    if (cmd[1][0] == '#') {
        Channel* ChannelToChat = server.getChannel(cmd[1]);
        if (ChannelToChat == NULL) {
            if (sendErs)
                sendError(fd, 401, server.getClient(fd).getNick(), cmd[1]);
            return;
        }
        if (!ChannelToChat->isMember(fd) && !ChannelToChat->getNoExternalMsgs()) {
            if (sendErs)
                sendError(fd, 404, server.getClient(fd).getNick(), ChannelToChat->getcName());
            return;
        }
        std::string msg = ":" + server.getClient(fd).getNick() + " " + cmd[0] + " " + ChannelToChat->getcName() + " " + cmd[2] + " \r\n";
        ChannelToChat->broadcast(fd, msg);
    } else if (server.isValidNick(cmd[1])) {
        Client* ClientToChat = server.getClientByNick(cmd[1]);
        if (ClientToChat == NULL) {
            if (sendErs)
                sendError(fd, 401, server.getClient(fd).getNick(), cmd[1]);
            return;
        }
        std::string msg = ":" + server.getClient(fd).getNick() + " " + cmd[0] + " " + ClientToChat->getNick() + " " + cmd[2] + " \r\n";
        dvais::sendMessage(ClientToChat->getFd(), msg);
    }
}

void Command::commandTopic(Server &server, int fd, const std::string &command)
{
    std::istringstream iss(command);
    std::string cmd, channelName;
    iss >> cmd >> channelName;
    std::string topic = dvais::extractTopic(iss);
    std::string nick = server.getClient(fd).getNick();

    if (channelName.empty() || channelName[0] != '#') {
        sendError(fd, 403, nick, channelName); // No such channel
        return;
    }
    Channel* channel = server.getChannel(channelName);
    if (!channel) {
        sendError(fd, 403, nick, channelName); // No such channel
        return;
    }
    if (!channel->isMember(fd)) {
        sendError(fd, 442, nick, channelName); // Not on channel
        return;
    }
    if (topic.empty())
    {
        std::string currentTopic = channel->getTopic();
        if (currentTopic.empty()) {
            sendError(fd, 331, nick, channelName); // No topic is set
        } else {
            dvais::sendMessage(fd, ":ircserv 332 " + nick + " " + channelName + " :" + currentTopic + "\r\n");

            std::ostringstream oss;
            oss << channel->getTopicSetTime();
            dvais::sendMessage(fd, ":ircserv 333 " + nick + " " + channelName + " " + channel->getTopicSetter() + " " + oss.str() + "\r\n");
        }
    } 
    else 
    {
        if (!channel->isOperator(fd) && channel->getTopicRestricted()) {
            sendError(fd, 482, nick, channelName); 
            return;
        }
        if (topic.empty()) {
            channel->clearTopic();
            channel->broadcast(-1, ":" + nick + " TOPIC " + channelName + " :\r\n");
        } else {
            channel->setTopic(topic, nick);
            channel->broadcast(-1, ":" + nick + " TOPIC " + channelName + " :" + topic + "\r\n");
        }
    }
}

void Command::commandNames(Server &server, int fd, const std::string &command)
{
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    std::string client_nick = server.getClient(fd).getNick();
    if (tokens.size() < 2) {
        return;
    }
    Channel* channel = server.getChannel(tokens[1]);
    if (!channel) {
        sendError(fd, 403, client_nick, tokens[1]);
        return;
    }
    std::string namesList = dvais::buildNamesList(server, channel);
    dvais::sendMessage(fd, ":ircserv 353 " + client_nick + " = " + channel->getcName() + " :" + namesList + "\r\n");
    dvais::sendMessage(fd, ":ircserv 366 " + client_nick + " " + channel->getcName() + " :End of /NAMES list\r\n");
}

void Command::commandQuit(Server &server, int fd, const std::string &command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    
    std::string quitMessage = dvais::extractTopic(iss);
    if (quitMessage.empty())
        quitMessage = "Client Quit";
    
    std::string nick = server.getClient(fd).getNick();
    std::string user = server.getClient(fd).getUser();
    std::string host = server.getClient(fd).getIp();
    std::string prefix = ":" + nick + "!" + user + "@" + host;
    std::string msg = prefix + " QUIT :" + quitMessage + "\r\n";
    server.broadcastAll(fd, msg);
    server.rmClient(fd);
}

void Command::commandInvite(Server &server, int fd, const std::string &command) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    std::string client_nick = server.getClient(fd).getNick();

    Channel* channel = server.getChannel(tokens[2]);
    if (!channel) {
        sendError(fd, 403, "", tokens[2]);
        return;
    }
    if (channel->isMember(fd) == false) {
        sendError(fd, 442, "", channel->getcName());
        return;
    }
    if (channel->getChannelType() == true && channel->isOperator(fd) == false) {
        sendError(fd, 482, "", channel->getcName());
        return;
    }
    Client* target_client = server.getClientByNick(tokens[1]);
    if (target_client && channel->isMember(target_client->getFd()) == true) {
        sendError(fd, 443, tokens[1], channel->getcName());
        return;
    } 
    else if (target_client && channel->isMember(target_client->getFd()) == false)
    {
        if (channel->isInvited(target_client->getFd()))
            return;
        std::string RPL_INVITING = ":ircserv " + tokens[1] + " " + channel->getcName() + "\r\n";
        std::string user = server.getClient(fd).getUser();
        std::string host = server.getClient(fd).getIp();
        std::string prefix = ":" + client_nick + "!" + user + "@" + host;
        std::string msg = prefix + " INVITE " + tokens[1] + " " + tokens[2] + "\r\n";
        dvais::sendMessage(fd, RPL_INVITING);
        dvais::sendMessage(target_client->getFd(), msg);
        channel->addInvited(target_client->getFd());
    }
}

void Command::commandKick(Server &server, int fd, const std::string &command) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    Client &client = server.getClient(fd);
    const std::string &client_nick = client.getNick();
    if (tokens.size() < 3) {
        sendError(fd, 461, client_nick, tokens[0]); // ERR_NEEDMOREPARAMS
        return;
    }
    // If a fourth token is present and begins with ':', remove it; otherwise, default to the target nick.
    std::string comment = (tokens.size() >= 4 && !tokens[3].empty() && tokens[3][0] == ':') ? tokens[3] : ":" + tokens[2];

    Channel* targetChannel = server.getChannel(tokens[1]);
    if (!targetChannel) {
        sendError(fd, 403, client_nick, tokens[1]); // ERR_NOSUCHCHANNEL
        return;
    }
    if (targetChannel->isMember(fd) == false) {
        sendError(fd, 442, "", targetChannel->getcName()); // ERR_NOTONCHANNEL
        return;
    }
    if (targetChannel->isOperator(fd) == false) {
        sendError(fd, 482, "", targetChannel->getcName()); // ERR_CHANOPRIVSNEEDED
        return;
    }
    Client* targetClient = server.getClientByNick(tokens[2]);
    if (!targetClient || !targetChannel->isMember(targetClient->getFd())) {
        sendError(fd, 441, tokens[2], targetChannel->getcName()); // ERR_USERNOTINCHANNEL
        return;
    }
    std::string user = client.getUser();
    std::string host = client.getIp();
    std::string kickMsg = ":" + client_nick + "!" + user + "@" + host + \
                            " KICK " + targetChannel->getcName() + \
                            " " + targetClient->getNick() + " " + comment + "\r\n";
    targetChannel->broadcast(-1, kickMsg);
    targetChannel->rmClient(targetClient->getFd());
}


void Command::executeCommand(Server &server, int fd, const std::string &command) {
    if (command.find("CAP ") == 0) {
        commandCap(fd, command);
    } else if (command.find("NICK") == 0) {
        commandNick(server, fd, command);
    } else if (command.find("USER ") == 0) {
        commandUser(server, fd, command);
    } else if (command.find("JOIN ") == 0) {
        commandJoin(server, fd, command);
    } else if (command.find("PING ") == 0) {
        commandPing(fd, command);
    } else if (command.find("INVITE ") == 0) {
        commandInvite(server, fd, command);
    } else if (command.find("MODE ") == 0) {
        commandMode(server, fd, command);
    } else if (command.find("KICK ") == 0) {
        commandKick(server, fd, command);
    } else if (command.find("PASS ") == 0) {
        commandPass(server, fd, command);
    } else if (command.find("PART ") == 0) {
        commandPart(server, fd, command);
    } else if (command.find("PRIVMSG ") == 0) {
        commandMsg(server, fd, command, true);
    } else if (command.find("NOTICE ") == 0) {
        commandMsg(server, fd, command, false);
    } else if (command.find("NAMES ") == 0) {
        commandNames(server, fd, command);
    } else if (command.find("WHOIS") == 0) {
        commandWhois(server, fd, command);
    } else if (command.find("WHO") == 0) {
        commandWho(server, fd, command);
    } else if (command.find("TOPIC") == 0) {
        commandTopic(server, fd, command);
    } else if (command.find("QUIT") == 0) {
        commandQuit(server, fd, command);
    } else {
        sendError(fd, 421, server.getClient(fd).getNick(), command.substr(0, command.find(" ")));
    }
}
