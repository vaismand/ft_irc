#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstdlib>
#include "Server.hpp"
#include "Tools.hpp"

class Server;

class Command {
public:
    // Constructors
    Command();
    Command(const Command &src);
    Command &operator=(const Command &src);
    ~Command();

    // Methods
    void commandCap(int fd, const std::string &command);
    void commandNick(Server &server, int fd, const std::string &command);
    void commandUser(Server &server, int fd, const std::string &command);
    void commandJoin(Server &server, int fd, const std::string &command);
    void commandPart(Server &server, int fd, const std::string &command);
    void commandMode(Server &server, int fd, const std::string &command);
    void commandKick(Server &server, int fd, const std::string &command);
    void commandPing(int fd, const std::string &command);
    void commandPass(Server &server, int fd, const std::string &command);
    void commandMsg(Server &server, int fd, const std::string &command, bool sendErs);
    void commandNames(Server &server, int fd, const std::string &command);
    void commandWhois(Server &server, int fd, const std::string &command);
    void commandWho(Server &server, int fd, const std::string &command);
    void commandTopic(Server &server, int fd, const std::string &command);
    void commandQuit(Server &server, int fd, const std::string &command);
    void commandInvite(Server &server, int fd, const std::string &command);
    void executeCommand(Server &server, int fd, const std::string &command);
    void sendError(int fd, int errorCode, const std::string &nick, const std::string &command);

private:
    // Attributes
    std::map<int, std::string> errorMap;
    std::string getErrorMessage(int errorCode, const std::string &nick, const std::string &command = "");
    void initErrorMap();
    void printChannelWelcome(Server &server, Client &client, Channel &channel, bool isnew);
    void partClientAll(Server &server, Client &client, std::vector<std::string> channels, std::string reason);
};
