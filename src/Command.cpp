#include "../inc/Command.hpp"
#include "../inc/Server.hpp"

Command::Command()
{
    initErrorMap();
}

Command::Command(const Command &obj)
{
    (void)obj;
}

Command &Command::operator=(const Command &rhs)
{
    (void)rhs;
    return *this;
}

Command::~Command() {}

/**
 * @brief Handles capability negotiation for IRC clients by processing CAP command.
 * If client requests "LS", a list of supported capabilities is returned.
 * If client requests "REQ", server refuses the requested capabilities.
 * "END" terminates the capability negotiation.
 */
void Command::commandCap(Server &server, int fd, const std::vector<std::string> &cmd)
{
    if (cmd.size() < 2) 
        sendError(fd, 421, "", "CAP"); // ERR_UNKNOWNCOMMAND
    std::string nick = server.getClientNick(fd);
    if (cmd[1] == "LS") {
        dvais::sendMessage(fd, "CAP " + nick + " LS :\r\n");
        return;
    }
    if (cmd[1] == "REQ") {
        if (cmd.size() < 3)
            dvais::sendMessage(fd, "CAP " + nick + " NAK :No capability specified\r\n");
        else
            dvais::sendMessage(fd, "CAP " + nick + " NAK " + cmd[2] + "\r\n");
        return;
    }
    if (cmd[1] == "END") {
        return;
    }
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
void Command::commandNick(Server &server, int fd, const std::vector<std::string> &tokens) {
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
    if (nickname.size() > MAX_DEFAULT_LEN)
        nickname.resize(MAX_DEFAULT_LEN);
    std::string user = client.getUser();
    std::string host = client.getIp();
    std::string msg = ":" + currentNick + "!" + user + "@" + host + " NICK :" + nickname + "\r\n";
    client.setNick(nickname);
    if (client.getStatus() == REGISTERED)
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
void Command::commandUser(Server &server, int fd, const std::vector<std::string> &tokens) {
    Client &client = server.getClient(fd);
    const std::string &nick = client.getNick();
    if (tokens.size() < 5 || tokens[1].empty() || tokens[4].empty() || \
    (tokens[4][0] == ':' && tokens[4].size() == 1)) {
        sendError(fd, 461, nick, "USER"); // ERR_NEEDMOREPARAMS
        return;
    }
    if (client.getStatus() == REGISTERED) {
        sendError(fd, 462, nick, ""); // ERR_ALREADYREGISTERED
        return;
    }
    std::string username = tokens[1];
    if (username.size() > MAX_DEFAULT_LEN)
        username.resize(MAX_DEFAULT_LEN);

    std::string realname = tokens[4];
    if (realname[0] == ':')
        realname = realname.substr(1);
    if (!server.isValidNick(username)) {
        executeCommand(server, fd, "NOTICE " + client.getNick() + " :*** Your username is invalid. " \
                            "Please make sure that your username contains only alphanumeric characters.\r\n");
        dvais::sendMessage(fd, "ERROR :Closing Link: Invalid username [~ " + client.getUser() + "]\r\n");
        server.rmClient(fd);
        return;
    }
    client.setUser(username);
    client.setRealName(realname);
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
 * @throws ERR_BADCHANMASK (476) - if the max channel name length is exceeded.
 */
void Command::commandJoin(Server &server, int fd, const std::vector<std::string> &tokens) {
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
        partClientAll(server, client, client.getChannelList(), "");
        channels.erase(channels.begin());
        if (!keys.empty())
            keys.erase(keys.begin());
    }
    for (std::size_t i = 0; i < channels.size(); i++) {
        if (channels[i].find("#") != 0) {
            sendError(fd, 403, nick, channels[i]); // ERR_NOSUCHCHANNEL
            continue;
        }
        if (channels[i].size() > MAX_CHAN_LEN) {
            sendError(fd, 476, nick, channels[i]); // ERR_BADCHANMASK
            continue;
        }
        if (client.getChannelList().size() >= server.getChannelLimit()) {
            sendError(fd, 405, nick, channels[i]); // ERR_TOOMANYCHANNELS
            continue;
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
            std::string providedKey = (i < keys.size()) ? keys[i] : "";
            if (!ChannelToJoin->getcKey().empty() && providedKey != ChannelToJoin->getcKey()) {
                sendError(fd, 475, nick, channels[i]); // ERR_BADCHANNELKEY 
                continue;
            }
        }
        ChannelToJoin->addClientToChannel(fd);
        std::string user = client.getUser();
        std::string host = client.getIp();
        client.addChannelToList(ChannelToJoin->getcName());
        std::string prefix = ":" + nick + "!" + user + "@" + host;
        ChannelToJoin->broadcast(-1, prefix + " " + tokens[0] + " " + channels[i] + "\r\n");
        printChannelWelcome(server, client, *ChannelToJoin, newChannel);
        if (ChannelToJoin->isInvited(fd))
        ChannelToJoin->rmInvited(fd);
    }
}

/**
 * @brief Handles the PART command in the IRC server.
 * Processes a comma-separated list of channels from which the client wants to part.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no channel is provided.
 * @throws ERR_NOSUCHCHANNEL (403) - if a specified channel does not exist.
 * @throws ERR_NOTONCHANNEL (442) - if the client is not a member of a specified channel.
 */
void Command::commandPart(Server &server, int fd, const std::vector<std::string> &tokens) {
    Client &client = server.getClient(fd);
    const std::string &nick = client.getNick();
    if (tokens.size() < 2) {
        sendError(fd, 461, nick, "PART"); // ERR_NEEDMOREPARAMS 
        return;
    }
    std::string reason = (tokens.size() >= 3 && !tokens[2].empty() && tokens[2][0] == ':') ? tokens[2] : "";

    std::vector<std::string> channels = dvais::split(tokens[1], ',');
    std::vector<std ::string> validChannels;
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++) {
        Channel* channelToPart = server.getChannel(*it);
        if (!channelToPart) {
            sendError(fd, 403, nick, *it); // ERR_NOSUCHCHANNEL
            continue;
        }
        if (!channelToPart->isMember(fd)) {
            sendError(fd, 442, nick, *it); // ERR_NOTONCHANNEL 
            continue;
        }
        validChannels.push_back(*it);
    }
    if (!validChannels.empty())
        partClientAll(server, client, channels, reason);
}

/**
 * @brief Handles the WHOIS command in the IRC server.
 * function retrieves information about that target nick. It sends a series of numeric replies:
 * - RPL_WHOISUSER (311): Provides basic user info (nickname, username, hostname).
 * - RPL_WHOISSERVER (312): Provides server info the target is connected to.
 * - RPL_WHOISCHANNELS (319): Provides info about the channels client is in.
 * - RPL_ENDOFWHOIS (318): End of the WHOIS reply.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no target nickname is provided.
 * @throws ERR_NOSUCHNICK (401) - if the specified target nickname does not exist.
 */
void Command::commandWhois(Server &server, int fd, const std::vector<std::string> &tokens) {
    Client &client = server.getClient(fd);
    const std::string &requesterNick = client.getNick();
    std::string targetNick = tokens[1];

    if (targetNick.empty()) {
        sendError(fd, 461, requesterNick, "WHOIS"); // ERR_NEEDMOREPARAMS 
        return;
    }
    Client* target = server.getClientByNick(targetNick);
    if (!target) {
        sendError(fd, 401, requesterNick, targetNick); // ERR_NOSUCHNICK
        return;
    }
    if (target->isInvisible())
    {
        if (target->getFd() != fd && !server.shareChannel(fd, target->getFd()))
        {
            sendError(fd, 401, requesterNick, targetNick); // ERR_NOSUCHNICK
            return;
        }
    }
    std::ostringstream oss;
    oss << ":ircserv 311 " << requesterNick << " " << target->getNick() <<  " ~" << target->getUser() 
        << " " << target->getIp() << " * :" << target->getRealName() << "\r\n";
    dvais::sendMessage(fd, oss.str());

    oss.str("");
    oss << ":ircserv 312 " << requesterNick << " " << target->getNick() << " ircserv" << " :Vienna, AT\r\n";
    dvais::sendMessage(fd, oss.str());
    
    oss.str("");
    if (!target->getChannelList().empty())
{
    oss << ":ircserv 319 " << requesterNick << " " << target->getNick() << " :";
    std::vector<std::string> channelList = target->getChannelList();
    for (std::vector<std::string>::iterator it = channelList.begin(); it != channelList.end(); it++)
    {
        Channel* channel = server.getChannel(*it);
        if (!channel)
            continue;
        if (channel->isOperator(target->getFd()))
            oss << "@";
        oss << *it << " ";
    }
    oss << "\r\n";
    dvais::sendMessage(fd, oss.str());
}
    oss.str("");
    oss << ":ircserv 318 " << requesterNick << " " << target->getNick() << " :End of WHOIS list\r\n";
    dvais::sendMessage(fd, oss.str());
}

/**
 * @brief Handles the WHO command in the IRC server. 
 * Can be used with a target nickname or a channel name.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no target nickname is provided.
 * @throws ERR_NOSUCHCHANNEL (403) - if the specified channel does not exist.
 * @throws ERR_NOSUCHNICK (401) - if the specified target nickname does not exist.
 */
void Command::commandWho(Server &server, int fd, const std::vector<std::string> &tokens) {
    std::string requesterNick = server.getClient(fd).getNick();

    if (tokens.size() < 2) {
        sendError(fd, 461, requesterNick, "WHO"); // ERR_NEEDMOREPARAMS
        return;
    }
    std::string target = tokens[1];
    if (target[0] == '#')
    {
        Channel* channel = server.getChannel(target);
        if (!channel) {
            sendError(fd, 403, requesterNick, target); // ERR_NOSUCHCHANNEL
            return;
        }
    const std::vector<int>& joined = server.getChannel(target)->getJoined();
    for (size_t i = 0; i < joined.size(); ++i)
    {
        Client &c = server.getClient(joined[i]);
        if (c.isInvisible())
        {
            continue;
        }
        std::ostringstream reply;
        reply << ":ircserv 352 " << requesterNick << " " << target << " "
                << c.getUser() << " " << c.getIp() << " ircserv "
                << c.getNick() << " H :0 " << c.getUser() << "\r\n";
        dvais::sendMessage(fd, reply.str());
    }
    std::ostringstream end;
    end << ":ircserv 315 " << requesterNick << " " << target << " :End of WHO list\r\n";
    dvais::sendMessage(fd, end.str());
    }
    else
    {
        Client *client = server.getClientByNick(target);
        if (!client) {
            sendError(fd, 401, requesterNick, target); // ERR_NOSUCHNICK
            return;
        }
        if (client->isInvisible())
        {
            if (client->getFd() != fd && !server.shareChannel(fd, client->getFd()))
            {
                sendError(fd, 401, requesterNick, target); // ERR_NOSUCHNICK
                return; 
            }
        }
        std::ostringstream reply;
        reply << ":ircserv 352 " << requesterNick << " " << target << " "
                << client->getUser() << " " << client->getIp() << " ircserv "
                << client->getNick() << " H :0 " << client->getUser() << "\r\n";
        dvais::sendMessage(fd, reply.str());
        std::ostringstream end;
        end << ":ircserv 315 " << requesterNick << " " << target << " :End of WHO list\r\n";
        dvais::sendMessage(fd, end.str());
    }
}

/**
 * @brief Handles the MODE command in the IRC server.
 * Can be used to set user modes or channel modes.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no target is provided.
 * @throws ERR_NOSUCHNICK (401) - if the target nickname does not exist.
 * @throws ERR_NOSUCHCHANNEL (403) - if the specified channel does not exist.
 * @throws ERR_NOTONCHANNEL (442) - if the client is not a member of the specified channel.
 * @throws ERR_KEYSET (467) - if the channel is set +k and the client is not a member.
 * @throws ERR_CHANOPRIVSNEEDED (482) - if the client is not a channel operator.
 * @throws ERR_UNKNOWNMODE (472) - if the mode is unknown.
 * @throws ERR_CHANOPRIVSNEEDED (482) - if the client is not a channel operator.
 * @throws ERR_NEEDMOREPARAMS (421) - if unknown command.
 * 
 */
void Command::commandMode(Server &server, int fd, const std::vector<std::string> &tokens )
{
    std::string nick = server.getClient(fd).getNick();

    if (tokens.size() < 2) {
        sendError(fd, 461, nick, "MODE"); // ERR_NEEDMOREPARAMS
        return;
    }
    std::string target = tokens[1];
    if (target == nick)
        handleUserMode(server, fd, tokens);
    else if (target[0] == '#' || target[0] == '&')
        handleChannelMode(server, fd, tokens);
    else
        sendError(fd, 421, nick, "MODE"); // ERR_UNKNOWNCOMMAND
}

/**
 * @brief Handles the PING command in the IRC server.
 *
 * @throws ERR_NEEDMOREPARAMS (461) - if no servername is provided.
 */
void Command::commandPing(int fd, const std::vector<std::string> &tokens) {
    if (tokens.size() < 2) {
        sendError(fd, 461, "", "PING"); // ERR_NEEDMOREPARAMS
        return;
    }
    std::string token = tokens[1];
    dvais::sendMessage(fd, "PONG " + token + "\r\n");
}

/**
 * @brief Handles the PONG received from client and update his Activity time.
 *
 * @throws ERR_NEEDMOREPARAMS (461) - if no servername is provided.
 */
void Command::commandPong(Server &server, int fd, const std::vector<std::string> &tokens) {
    if (tokens.size() < 2) {
        sendError(fd, 461, "", "PING"); // ERR_NEEDMOREPARAMS
        return;
    }
    std::string token = tokens[1];
    Client &client = server.getClient(fd);
    client.setLastActivity(time(NULL));
    client.setPingSent(false);
}

/**
 * @brief Handles the PASS command in the IRC server. If the provided password is correct,
 * it sets the passAccepted flag to true. If not, it sends an error reply and removes the client.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no password is provided.
 * @throws ERR_PASSWDMISMATCH (464) - if the provided password is incorrect.
 * @throws ERR_ALREADYREGISTERED (462) - if the client is already registered.
 */
void Command::commandPass(Server &server, int fd, const std::vector<std::string> &tokens) {
    if (tokens.size() < 2) {
        sendError(fd, 461, "", "PASS"); // ERR_NEEDMOREPARAMS
        return;
    }
    std::string providedPass = tokens[1];
    if (server.getClient(fd).getPassAccepted()) {
        sendError(fd, 462, "", ""); // ERR_ALREADYREGISTERED
    } else if (providedPass != server.getPass()) {
        sendError(fd, 464, "", ""); // ERR_PASSWDMISMATCH
        server.rmClient(fd);
    } else {
        server.getClient(fd).setPassAccepted(true);
    }
}

/**
 * @brief Handles PRIVMSG and NOTICE commands in the IRC server.
 * Processes a message from a client to a targgetPassAccet (channel or user). If sendErs flag is true, 
 * it sends error replies (PRIVMSG):
 *
 * @throws ERR_NORECIPIENT (411) - if no recipient is provided.
 * @throws ERR_NOTEXTTOSEND (412) - if the message text is missing.
 * @throws ERR_NOSUCHNICK (401) - if the target nick or channel doesn't exist.
 * @throws ERR_CANNOTSENDTOCHAN (404) - if the client is not allowed to send to the channel.
 */
void Command::commandMsg(Server &server, int fd, const std::vector<std::string> &cmd, bool sendErs) {
    Client &client = server.getClient(fd);
    const std::string &nick = client.getNick();
    if (cmd.size() < 3) {
        if (sendErs)
            sendError(fd, 411, nick, cmd[0]); // ERR_NORECIPIENT
        return;
    }
    if (cmd[2].empty() || cmd[2][0] != ':') {
        if (sendErs)
            sendError(fd, 412, nick, "PRIVMSG"); // ERR_NOTEXTTOSEND
        return;
    }
    if (cmd[1][0] == '#') {
        Channel* ChannelToChat = server.getChannel(cmd[1]);
        if (ChannelToChat == NULL) {
            if (sendErs)
                sendError(fd, 401, nick, cmd[1]); // ERR_NOSUCHNICK
            return;
        }
        if (!ChannelToChat->isMember(fd) && ChannelToChat->getNoExternalMsgs()) {
            if (sendErs)
                sendError(fd, 404, nick, ChannelToChat->getcName()); // ERR_CANNOTSENDTOCHAN
            return;
        }
        std::string msg = ":" + nick + " " + cmd[0] + " " + ChannelToChat->getcName() + " " + cmd[2] + " \r\n";
        ChannelToChat->broadcast(fd, msg);
    } else if (server.isValidNick(cmd[1])) {
        Client* targetClient = server.getClientByNick(cmd[1]);
        if (targetClient == NULL) {
            if (sendErs)
                sendError(fd, 401, server.getClient(fd).getNick(), cmd[1]);
            return;
        }
        std::string msg = ":" + nick + " " + cmd[0] + " " + targetClient->getNick() + " " + cmd[2] + " \r\n";
        dvais::sendMessage(targetClient->getFd(), msg);
    }
}

/**
 * @brief Handles the TOPIC command in the IRC server. If no topic is provided, it sends the current topic.
 * If a topic is provided, it sets the new topic and broadcasts it to all members.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no channel is provided.
 * @throws ERR_NOSUCHCHANNEL (403) - if the specified channel does not exist.
 * @throws ERR_NOTONCHANNEL (442) - if the client is not a member of the specified channel.
 * @throws ERR_CHANOPRIVSNEEDED (482) - if the channel is invite-only and the client is not an operator.
 */
void Command::commandTopic(Server &server, int fd, const std::vector<std::string> &cmd)
{
    Client &client = server.getClient(fd);
    const std::string &nick = client.getNick();

    if (cmd.size() < 2)
    {
        sendError(fd, 461, nick, "TOPIC"); // ERR_NEEDMOREPARAMS
        return;
    }
    std::string channelName = cmd[1];
    if (channelName.empty() || channelName[0] != '#')
    {
        sendError(fd, 403, nick, channelName); // ERR_NOSUCHCHANNEL
        return;
    }
    Channel* channel = server.getChannel(channelName);
    if (!channel)
    {
        sendError(fd, 403, nick, channelName); // ERR_NOSUCHCHANNEL
        return;
    }
    if (!channel->isMember(fd))
    {
        sendError(fd, 442, nick, channelName); // ERR_NOTONCHANNEL
        return;
    }
    if (cmd.size() < 3)
    {
        std::string currentTopic = channel->getcTopic();
        if (currentTopic.empty())
        {
            sendError(fd, 331, nick, channelName); // ERR_NOTOPIC
        }
        else
        {
            dvais::sendMessage(fd, ":ircserv 332 " + nick + " " + channelName + " :" + currentTopic + "\r\n");
            dvais::sendMessage(fd, ":ircserv 333 " + nick + " " + channelName + " "
                                   + dvais::topicSetterTime(channel->getTopicSetter(), channel->getTopicSetTime()) 
                                   + "\r\n");
        }
        return;
    }
    std::string rawTopic = cmd[2];
    if (!rawTopic.empty() && rawTopic[0] == ':')
        rawTopic.erase(0, 1);
    if (rawTopic.size() > MAX_TOPIC_LEN)
        rawTopic.resize(MAX_TOPIC_LEN);
    if (channel->getTopicRestricted() && !channel->isOperator(fd))
    {
        sendError(fd, 482, nick, channelName); // ERR_CHANOPRIVSNEEDED
        return;
    }
    if (rawTopic.empty())
    {
        channel->clearTopic();
        channel->broadcast(-1, ":" + nick + " TOPIC " + channelName + " :\r\n");
    }
    else
    {
        channel->setTopic(rawTopic, nick);
        channel->broadcast(-1, ":" + nick + " TOPIC " + channelName + " :" + rawTopic + "\r\n");
    }
}

/**
 * @brief Handles the NAMES command in the IRC server.
 * It shows a list of nicknames in a specific channel for members of the channel only.
 * Command needs channel name as argument. It sends two numeric replies:
 * - RPL_NAMREPLY (353): Contains the list of user nicknames in the channel.
 * - RPL_ENDOFNAMES (366): Indicates the end of the NAMES list.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if no channel is provided.
 */
void Command::commandNames(Server &server, int fd, const std::vector<std::string> &tokens)
{
    std::string client_nick = server.getClient(fd).getNick();
    if (tokens.size() < 2) {
        sendError(fd, 461, client_nick, "NAMES"); // ERR_NEEDMOREPARAMS
        return;
    }
    Channel* channel = server.getChannel(tokens[1]);
    if (channel) {
        std::string namesList = dvais::buildNamesList(server, channel, fd);
        dvais::sendMessage(fd, ":ircserv 353 " + client_nick + " = " + channel->getcName() + " :" + namesList + "\r\n");
    }
    dvais::sendMessage(fd, ":ircserv 366 " + client_nick + " " + tokens[1] + " :End of /NAMES list\r\n");
}
/**
 * @brief Handles the QUIT command in the IRC server. Can be sent with or without a quit message.
 * Broadcasts the QUIT message to all channels and removes the client from the server.
 * No numeric replies are sent.
 */
void Command::commandQuit(Server &server, int fd, const std::vector<std::string> &tokens) {
    if (tokens.size() < 2) {
        executeCommand(server, fd, "QUIT :Client Quit\r\n");
        return;
    }
    std::string reason = (tokens.size() >= 2 && !tokens[1].empty() && tokens[1][0] == ':') ? tokens[1] : "";
    
    std::string nick = server.getClient(fd).getNick();
    std::string user = server.getClient(fd).getUser();
    std::string host = server.getClient(fd).getIp();
    std::string prefix = ":" + nick + "!" + user + "@" + host;
    std::string msg = prefix + " QUIT " + reason + "\r\n";
    server.broadcastAll(fd, msg);
    server.getClient(fd).setHasQuit(true);
    server.rmClient(fd);
}

/**
 * @brief Handles the INVITE command in the IRC server. 
 * Sends an invitation to the target user for the given channel and notifies the inviter.
 * It sends back one numeric reply:
 * - RPL_INVITING (341): When target is successfully invited.
 *  
 * @throws ERR_NEEDMOREPARAMS (461) - if less than 3 parameters provided.
 * @throws ERR_NOSUCHCHANNEL (403) - if the specified channel does not exist.
 * @throws ERR_NOTONCHANNEL (442) - if the inviter is not on the channel.
 * @throws ERR_CHANOPRIVSNEEDED (482) - if the channel is invite-only and the inviter is not an operator.
 * @throws ERR_USERONCHANNEL (443) - if the target is already on the channel. 
 * @throws ERR_NOSUCHNICK (401) - if the target nickname does not exist.
 */
void Command::commandInvite(Server &server, int fd, const std::vector<std::string> &tokens) {
    std::string client_nick = server.getClient(fd).getNick();
    if (tokens.size() < 3) {
        sendError(fd, 461, client_nick, "INVITE"); // ERR_NEEDMOREPARAMS
        return;
    }

    Client* target_client = server.getClientByNick(tokens[1]);
    Channel* channel = server.getChannel(tokens[2]);

    if (!target_client) {
        sendError(fd, 401, client_nick, tokens[1]); // ERR_NOSUCHNICK
        return;
    }
    if (!channel) {
        sendError(fd, 403, client_nick, tokens[2]); // ERR_NOSUCHCHANNEL
        return;
    }
    if (channel->isMember(fd) == false) {
        sendError(fd, 442, client_nick, channel->getcName()); // ERR_NOTONCHANNEL
        return;
    }
    if (channel->getChannelType() == true && channel->isOperator(fd) == false) {
        sendError(fd, 482, client_nick, channel->getcName()); // ERR_CHANOPRIVSNEEDED
        return;
    }
    if (channel->isMember(target_client->getFd()) == true) {
        sendError(fd, 443, tokens[1], channel->getcName()); // ERR_USERONCHANNEL
        return;
    } 
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

/**
 * @brief Handles the KICK command in the IRC server.
 * Broadcasts the KICK message and removes the target from the channel.
 * 
 * @throws ERR_NEEDMOREPARAMS (461) - if fewer than 3 parameters are provided.
 * @throws ERR_NOSUCHCHANNEL (403) - if specified channel doesn't exist.
 * @throws ERR_NOTONCHANNEL (442) - if client isn't in the channel.
 * @throws ERR_CHANOPRIVSNEEDED (482) - if client lacks operator status.
 * @throws ERR_USERNOTINCHANNEL (441) - if target isn't in the channel.
 */
void Command::commandKick(Server &server, int fd, const std::vector<std::string> &tokens)
{
    Client &client = server.getClient(fd);
    const std::string &client_nick = client.getNick();

    if (tokens.size() < 3) {
        sendError(fd, 461, client_nick, tokens[0]); // ERR_NEEDMOREPARAMS
        return;
    }
    const std::string &channelName = tokens[1];
    const std::string &targetNick  = tokens[2];
    Channel* targetChannel = server.getChannel(channelName);
    if (!targetChannel) {
        sendError(fd, 403, client_nick, channelName); // ERR_NOSUCHCHANNEL
        return;
    }
    if (!targetChannel->isMember(fd)) {
        sendError(fd, 442, client_nick, channelName); // ERR_NOTONCHANNEL
        return;
    }
    if (!targetChannel->isOperator(fd)) {
        sendError(fd, 482, client_nick, channelName); // ERR_CHANOPRIVSNEEDED
        return;
    }
    Client* targetClient = server.getClientByNick(targetNick);
    if (!targetClient || !targetChannel->isMember(targetClient->getFd())) {
        sendError(fd, 441, targetNick, channelName); // ERR_USERNOTINCHANNEL
        return;
    }
    std::string comment;
    if (tokens.size() < 4) {
        comment = ":" + targetNick;
    } else {
        if (tokens[3] == ":") {
            comment = ":" + targetNick;
        } else if (!tokens[3].empty() && tokens[3][0] == ':') {
            comment = tokens[3];
        } else {
            comment = ":" + tokens[3];
        }
    }
    std::string user = client.getUser();
    std::string host = client.getIp();
    std::string kickMsg = ":" + client_nick + "!" + user + "@" + host
        + " KICK " + channelName + " " + targetClient->getNick() + " " + comment + "\r\n";
    targetChannel->broadcast(-1, kickMsg);
    targetChannel->rmClientFromChannel(targetClient->getFd());
}

void Command::executeCommand(Server &server, int fd, const std::string &cmd) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(cmd);
    if (tokens.empty())
        return;
    std::string command = tokens[0];

    if (command == "CAP") {
        commandCap(server, fd, tokens);
    } else if (command == "NICK") {
        commandNick(server, fd, tokens);
    } else if (command == "USER") {
        commandUser(server, fd, tokens);
    } else if (command == "JOIN") {
        commandJoin(server, fd, tokens);
    } else if (command == "PING") {
        commandPing(fd, tokens);
    } else if (command == "PONG") {
        commandPong(server, fd, tokens);
    } else if (command == "INVITE") {
        commandInvite(server, fd, tokens);
    } else if (command == "MODE") {
        commandMode(server, fd, tokens);
        updateAllModes(server, fd);
    } else if (command == "KICK") {
        commandKick(server, fd, tokens);
    } else if (command == "PASS") {
        commandPass(server, fd, tokens);
    } else if (command == "PART") {
        commandPart(server, fd, tokens);
    } else if (command == "PRIVMSG") {
        commandMsg(server, fd, tokens, true);
    } else if (command == "NOTICE") {
        commandMsg(server, fd, tokens, false);
    } else if (command == "NAMES") {
        commandNames(server, fd, tokens);
    } else if (command == "WHOIS") {
        commandWhois(server, fd, tokens);
    } else if (command == "WHO") {
        commandWho(server, fd, tokens);
    } else if (command == "TOPIC") {
        commandTopic(server, fd, tokens);
    } else if (command == "QUIT") {
        commandQuit(server, fd, tokens);
    } else {
        sendError(fd, 421, server.getClient(fd).getNick(), command.substr(0, command.find(" ")));
    }
}
