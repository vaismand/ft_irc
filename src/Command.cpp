/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 12:50:25 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/10 10:32:33 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Command.hpp"

Command::Command() {}

Command::Command(const Command &src) {
    (void)src;
}

void Command::commandCap(Server &server, int fd, const std::string &command)
{
    (void)server;

    if (command.find("CAP LS") == 0)
    {
        send(fd, "CAP * LS :multi-prefix away-notify\r\n", 36, 0);
        return;
    }

    if (command.find("CAP REQ") == 0)
    {
        send(fd, "CAP * ACK :multi-prefix away-notify\r\n", 38, 0);
        send(fd, "CAP END\r\n", 10, 0);  // End capability negotiation
        return;
    }
}


void Command::executeCommand(Server &server, int fd, const std::string &command)
{
    if (command.find("CAP ") == 0) {
        commandCap(server, fd, command);
    }
    if (command.find("NICK ") == 0) {
        commandNick(server, fd, command);
    }
    else if (command.find("USER ") == 0) {
        commandUser(server, fd, command);
    }
    else if (command.find("JOIN ") == 0) {
        commandJoin(server, fd, command);
    }
    else if (command.find("PING ") == 0) {
        commandPing(server, fd, command);
    }
    else {
        send(fd, "Error: Unknown command.\r\n", 24, 0);
    }
}

void Command::commandNick(Server &server, int fd, const std::string &command)
{
    std::string nickname = command.substr(5);
    server.getClient(fd).setNick(nickname);
    std::string msg = "Nickname set to " + nickname + "\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
}

void Command::commandUser(Server &server, int fd, const std::string &command)
{
	(void)command;
	(void)server;
    send(fd, "User registered.\r\n", 18, 0);
}

void Command::commandJoin(Server &server, int fd, const std::string &command)
{
    std::string channel = command.substr(5);
    server.addClientToChannel(fd, channel);
    std::string joinMsg = ":" + server.getClientNick(fd) + " JOIN " + channel + "\r\n";
    server.broadcastToChannel(channel, joinMsg);
}

void Command::commandPing(Server &server, int fd, const std::string &command)
{
	(void)server;
    std::string response = "PONG " + command.substr(5) + "\r\n";
    send(fd, response.c_str(), response.size(), 0);
}
