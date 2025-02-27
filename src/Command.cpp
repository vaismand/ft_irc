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
    errorMap[401] = "No such nick/channel";
    errorMap[404] = "Cannot send to channel";
    errorMap[411] = "No recipient given";
    errorMap[421] = "Unknown command";
    errorMap[432] = "Erroneous nickname";
    errorMap[433] = "Nickname is already in use";
    errorMap[451] = "You have not registered";
    errorMap[461] = "Not enough parameters";
    errorMap[464] = "Password incorrect";
    errorMap[475] = "Cannot join channel (invite only)";
    errorMap[403] = "No such channel";
    errorMap[412] = "No text to send";
    errorMap[442] = "Not on channel";
    errorMap[482] = "You're not channel operator";
    errorMap[331] = "No topic is set";
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

void Command::commandNick(Server &server, int fd, const std::string &command) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    if (tokens.size() < 2) {
        // Not enough parameters; send an error message for missing NICK parameter.
        sendError(fd, 461, server.getClient(fd).getNick(), "NICK");
        return;
    }
    std::string nickname = tokens[1]; //!! solve : problem by only allowing alphanumeric characters for nick
    std::string currentNick = server.getClient(fd).getNick();
    if (currentNick.empty())
        currentNick = "*";
    if (server.isNickInUse(nickname, fd)) {
        sendError(fd, 433, currentNick, nickname);
        return;
    }
    if (nickname.empty() || !isValidNick(nickname)) {
        sendError(fd, 432, "", nickname);
        return;
    }
    // Retrieve client infos.
    std::string user = server.getClient(fd).getUser();
    std::string host = server.getClient(fd).getIp();
    std::string msg = ":" + currentNick + "!" + user + "@" + host + " NICK :" + nickname + "\r\n";
    server.getClient(fd).setNick(nickname);
    dvais::sendMessage(fd, msg);
}

void Command::commandUser(Server &server, int fd, const std::string &command) {
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    if (tokens.size() < 2 || tokens[1].empty()) {
        // Not enough parameters; send an error message.
        sendError(fd, 461, server.getClient(fd).getNick(), "USER");
        return;
    }
    std::string username = tokens[1];
    server.getClient(fd).setUser(username);
    dvais::sendMessage(fd, "Username set to " + username + "\r\n");
}

bool Command::isValidNick(const std::string &nickname) {
    if (nickname.empty() || !isalpha(nickname[0]))
        return false;
    for (size_t i = 0; i < nickname.length(); ++i) {
        char c = nickname[i];
        if (!isalnum(c) && c != '-' && c != '_')
            return false;
    }
    return true;
}

void Command::commandJoin(Server &server, int fd, const std::string &command) {
    std::string nick = server.getClient(fd).getNick();
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    std::string channelName = tokens[1];

    if (channelName.find(",") != std::string::npos) {
        sendError(fd, 475, nick, channelName);
        return;
    }
    if (channelName.find(" ") != std::string::npos) {
        sendError(fd, 461, nick, "JOIN");
        return;
    }
    if (channelName.find("#") != 0) {
        sendError(fd, 403, nick, channelName);
        return;
    }
    if (server.getClient(fd).getStatus() != REGISTERED) {
        sendError(fd, 451, nick, "JOIN");
        return;
    }
    if (server.getClient(fd).getNick().empty()) {
        sendError(fd, 451, nick, "JOIN");
        return;
    }
    if (server.getClient(fd).getUser().empty()) {
        sendError(fd, 451, nick, "JOIN");
        return;
    }
    if (!server.getClient(fd).getPassAccepted()) {
        sendError(fd, 464, nick, "JOIN");
        return;
    }
    Channel* ChannelToJoin = server.getChannel(channelName);
    if (ChannelToJoin == NULL) {
        server.addChannel(fd, channelName, "");
        ChannelToJoin = server.getChannel(channelName);
    }
    ChannelToJoin->addClient(server.getClient(fd).getFd());
    std::string user = server.getClient(fd).getUser();
    std::string host = server.getClient(fd).getIp();
    ChannelToJoin->broadcast(-1, ":" + nick + "!" + user + "@" + host + " JOIN " + channelName + "\r\n");
    // server.printChannelWelcome(fd, nick, *ChannelToJoin);
    commandNames(server, fd, "NAMES " + ChannelToJoin->getcName());
}

void Command::commandPart(Server &server, int fd, const std::string &command)
{
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    std::string nick = server.getClient(fd).getNick();
    if (tokens.size() < 2) {
        sendError(fd, 461, nick, "PART");
        return;
    }
    std::string channelName = tokens[1];
    Channel* tmp = server.getChannel(channelName);
    if (!tmp) {
        sendError(fd, 421, nick, channelName);
        return;
    }
    if (!tmp->isMember(fd)) {
        sendError(fd, 442, nick, channelName);
        return;
    }
    std::string user = server.getClient(fd).getUser();
    std::string host = server.getClient(fd).getIp();
    std::string prefix = ":" + nick + "!" + user + "@" + host;
    std::string msg = prefix + " PART " + channelName + "\r\n";
    tmp->broadcast(fd, msg);
    dvais::sendMessage(fd, msg);
    tmp->rmClient(fd);
    if (tmp->getJoined().empty())
    {
        server.rmChannel(channelName);
    }
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

void Command::commandMode(Server &server, int fd, const std::string &command)
{
    std::vector<std::string> tokens = dvais::cmdtokenizer(command);
    std::string clientNick = server.getClient(fd).getNick();
    std::string target = (tokens.size() >= 2) ? tokens[1] : clientNick;
    
    if (target == clientNick)
    {
        dvais::sendMessage(fd, ":ircserv 221 " + clientNick + " :\r\n");
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
        server.removeClient(fd);
    } else {
        server.getClient(fd).setPassAccepted(true);
    }
}

void Command::commandPrivmsg(Server &server, int fd, const std::string &command) {
    std::vector<std::string> cmd = dvais::cmdtokenizer(command);
    if (cmd.size() < 3) {
        sendError(fd, 411, server.getClient(fd).getNick(), "PRIVMSG");
        return;
    }
    if (cmd[2].empty() || cmd[2][0] != ':') {
        sendError(fd, 412, server.getClient(fd).getNick(), "PRIVMSG");
        return;
    }
    //if recipient is a channel, broadcast to channel
    if (cmd[1][0] == '#') {
        Channel* ChannelToChat = server.getChannel(cmd[1]);
        if (ChannelToChat == NULL) {
            sendError(fd, 401, server.getClient(fd).getNick(), cmd[1]);
            return;
        }
        if (!ChannelToChat->isMember(fd)) {
            sendError(fd, 404, server.getClient(fd).getNick(), ChannelToChat->getcName());
            return;
        }
        std::string msg = ":" + server.getClient(fd).getNick() + " " + cmd[0] + " " + ChannelToChat->getcName() + " " + cmd[2] + " \r\n";
        ChannelToChat->broadcast(fd, msg);
    // If the target is a user (valid nick)
    } else if (isValidNick(cmd[1])) {
        Client* ClientToChat = server.getClientByNick(cmd[1]);
        if (ClientToChat == NULL) {
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
        if (!channel->isOperator(fd)) {
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

void Command::executeCommand(Server &server, int fd, const std::string &command) {
    if (command.find("CAP ") == 0) {
        commandCap(fd, command);
    } else if (command.find("NICK ") == 0) {
        commandNick(server, fd, command);
    } else if (command.find("USER ") == 0) {
        commandUser(server, fd, command);
    } else if (command.find("JOIN ") == 0) {
        commandJoin(server, fd, command);
    } else if (command.find("PING ") == 0) {
        commandPing(fd, command);
    } else if (command.find("MODE ") == 0) {
        commandMode(server, fd, command);
    } else if (command.find("PASS ") == 0) {
        commandPass(server, fd, command);
    } else if (command.find("PART ") == 0) {
        commandPart(server, fd, command);
    } else if (command.find("PRIVMSG ") == 0) {
        commandPrivmsg(server, fd, command);
    } else if (command.find("NAMES ") == 0) {
        commandNames(server, fd, command);
    } else if (command.find("WHOIS") == 0) {
        commandWhois(server, fd, command);
    } else if (command.find("TOPIC") == 0) {
        commandTopic(server, fd, command);
    } else if (command.find("QUIT") == 0) {
        server.removeClient(fd);
    } else {
        sendError(fd, 421, server.getClient(fd).getNick(), command.substr(0, command.find(" ")));
    }
}
