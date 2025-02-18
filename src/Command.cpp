/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 12:50:25 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/18 12:00:25 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Command.hpp"

Command::Command() {}

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

static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

void Command::commandCap(int fd, const std::string &command)
{
    if (command.find("CAP LS") == 0)
    {
        dvais::sendMessage(fd, "CAP * LS :multi-prefix away-notify\r\n");
        return;
    }
    if (command.find("CAP REQ") == 0)
    {
        dvais::sendMessage(fd, "CAP * ACK :multi-prefix away-notify\r\n");
        return;
    }
    if (command.find("CAP END") == 0)
    {
        return;
    }
    dvais::sendMessage(fd, "Error: Unknown CAP subcommand.\r\n");
}

void Command::commandNick(Server &server, int fd, const std::string &command)
{
    std::string nickname = command.substr(5);
    nickname = trim(nickname);
    if (nickname.empty() || nickname.length() > 9 || !isValidNick(nickname))
    {
        dvais::sendMessage(fd, ":ircserv 432 * " + nickname + " :Erroneous nickname\r\n");
        return;
    }

    server.getClient(fd).setNick(nickname);
    std::string msg = "Nickname set to " + nickname + "\r\n";
    dvais::sendMessage(fd, msg);
}

void Command::commandUser(Server &server, int fd, const std::string &command)
{
    std::string username = command.substr(5);
    username = trim(username);
    server.getClient(fd).setUser(username);
    std::string msg = "Username set to " + username + "\r\n";
    dvais::sendMessage(fd, msg);
}


bool Command::isValidNick(const std::string &nickname)
{
    if (nickname.empty() || !isalpha(nickname[0]))
        return false;
    for (size_t i = 0; i < nickname.length(); ++i)
    {
        char c = nickname[i];
        if (!isalnum(c) && c != '-' && c != '_')
            return false;
    }
    return true;
}

void Command::commandJoin(Server &server, int fd, const std::string &command)
{
    std::string nick = server.getClient(fd).getNick();
    std::string channelName = command.substr(5);

    if (channelName.find(",") != std::string::npos)
    {
        dvais::sendMessage(fd, ":ircserv 475 " + nick + " " + channelName + " :Cannot join channel (invite only)\r\n");
        return;
    }
    if (channelName.find(" ") != std::string::npos)
    {
        dvais::sendMessage(fd, ":ircserv 461 " + nick + " JOIN :Not enough parameters\r\n");
        return;
    }
    if (channelName.find("#") != 0)
    {
        dvais::sendMessage(fd, ":ircserv 403 " + nick + " " + channelName + " :No such channel\r\n");
        return;
    }
    if (server.getClient(fd).getStatus() != REGISTERED)
    {
        dvais::sendMessage(fd, ":ircserv 451 " + nick + " JOIN :You have not registered\r\n");
        return;
    }
    if (server.getClient(fd).getNick().empty())
    {
        dvais::sendMessage(fd, ":ircserv 451 " + nick + " JOIN :You have not set a nickname\r\n");
        return;
    }
    if (server.getClient(fd).getUser().empty())
    {
        dvais::sendMessage(fd, ":ircserv 451 " + nick + " JOIN :You have not set a username\r\n");
        return;
    }
    if (server.getClient(fd).getPassAccepted() == false)
    {
        dvais::sendMessage(fd, ":ircserv 464 " + nick + " JOIN :Password incorrect\r\n");
        return;
    }
    Channel* ChannelToJoin = server.getChannel(channelName);
    if (ChannelToJoin == NULL) {
        server.addChannel(channelName, "");
        ChannelToJoin = server.getChannel(channelName);
    }
    ChannelToJoin->addClient(server.getClient(fd).getFd());
    std::string msg = ":" + nick + " JOIN " + channelName + "\r\n";
    ChannelToJoin->broadcast(msg);
}

void Command::commandPart(Server &server, int fd, const std::string &command)
{
    std::string nick = server.getClient(fd).getNick();
    std::string channelName = command.substr(5);
    Channel* tmp = server.getChannel(channelName);
    if (!tmp) {
        std::string reply = ":ircserv 421 " + channelName + ": No such channel\r\n";
        dvais::sendMessage(fd, reply);
        return;
    }
    if(!tmp->isMember(fd)) {
        dvais::sendMessage(fd, ":ircserv 442 " + channelName + ": You're not on that channel\r\n");
        return;
    }
    std::string msg = ":" + nick + " PART " + channelName + "\r\n";
    tmp->broadcast(msg);
    tmp->rmClient(fd);
}

void Command::commandMode(Server &server, int fd, const std::string &command)
{
    (void)command;
    std::string nick = server.getClient(fd).getNick();
    std::string reply = ":ircserv 421 " + nick + " MODE :Not implemented\r\n";
    dvais::sendMessage(fd, reply);
}

void Command::commandPing(int fd, const std::string &command)
{
    std::string servername = command.substr(5);
    std::string msg = "PONG " + servername + "\r\n";
    dvais::sendMessage(fd, msg);
}

void Command::commandPass(Server &server, int fd, const std::string &command)
{
    std::string providedPass = command.substr(5);
    if (providedPass != server.getPass())
    {
        dvais::sendMessage(fd, ":ircserv 464 * :Password incorrect\r\n");
        server.removeClient(fd);
    } 
    else
    {
        server.getClient(fd).setPassAccepted(true);
    }
}
 static std::vector<std::string> cmdtokenizer(const std::string& command) {
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;

    if(command.empty())
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



void Command::commandPrivmsg(Server &server, int fd, const std::string &command)
{
    std::vector<std::string> cmd = cmdtokenizer(command);
    if (cmd.empty()) {
        std::cout << "place error msg here" << std::endl; // error handling needs to be solved.
        return;
    }
    //Check if Channel has a #, if Channel exists and if Client is in Channel and if it has the needed rights.
    std::size_t chanPos = cmd[1].find("#");
    if (chanPos != 0 ) {
        std::cout << "place error msg here" << std::endl; // error handling needs to be solved.
        return;
    }
    Channel* ChannelToChat = server.getChannel(cmd[1]);
    if (ChannelToChat == NULL) {
        std::cout << "place error msg here" << std::endl; // error handling needs to be solved.
        return;
    }
    if(!ChannelToChat->isMember(fd)) {
        dvais::sendMessage(fd, ":ircserv 442 " + ChannelToChat->getcName() + ": You're not on that channel\r\n");
        return;
    }
    //Create the message to be broadcasted to all members of the channel.
    std::size_t msgPos = cmd[2].find_first_of(":");
    if (msgPos != 0) {
        std::cout << "place error msg here" << std::endl; // error handling needs to be solved.
        return;
    }
    std::string msg = ":" + server.getClient(fd).getNick() + " " + cmd[0] + " " + ChannelToChat->getcName() + " " + cmd[2] + " \r\n";
    ChannelToChat->broadcast(msg);
}

void Command::executeCommand(Server &server, int fd, const std::string &command)
{
    if (command.find("CAP ") == 0)
    {
        commandCap(fd, command);
    }
    else if (command.find("NICK ") == 0)
    {
        commandNick(server, fd, command);
    }
    else if (command.find("USER ") == 0)
    {
        commandUser(server, fd, command);
    }
    else if (command.find("JOIN ") == 0)
    {
        commandJoin(server, fd, command);
    }
    else if (command.find("PING ") == 0)
    {
        commandPing(fd, command);
    }
    else if (command.find("MODE ") == 0)
    {
        commandMode(server, fd, command);
    }
    else if (command.find("PASS ") == 0)
    {
        commandPass(server, fd, command);
    }
    else if (command.find("PART ") == 0)
    {
        commandPart(server, fd, command);
    }
    else if (command.find("PRIVMSG ") == 0)
    {
        commandPrivmsg(server, fd, command);
    }
    else
    {
        std::string nick = server.getClient(fd).getNick();
        std::string reply = ":ircserv 421 " + nick + " " + command.substr(0, command.find(" ")) + " :Unknown command\r\n";
        dvais::sendMessage(fd, reply);
    }
}
