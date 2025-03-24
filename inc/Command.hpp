#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstdlib>
#include "Tools.hpp"
#include "Client.hpp"

class Server;

class Command
{
    public:
        // Constructors
        Command();
        Command(const Command &src);
        Command &operator=(const Command &src);
        ~Command();

        // Methods
        void commandCap(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandNick(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandUser(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandJoin(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandPart(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandMode(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandKick(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandPing(int fd, const std::vector<std::string> &tokens);
        void commandPong(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandPass(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandMsg(Server &server, int fd, const std::vector<std::string> &tokens, bool sendErs);
        void commandNames(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandWhois(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandWho(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandTopic(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandQuit(Server &server, int fd, const std::vector<std::string> &tokens);
        void commandInvite(Server &server, int fd, const std::vector<std::string> &tokens);
        void executeCommand(Server &server, int fd, const std::string &command);
        void sendError(int fd, int error, const std::string &nick, const std::string &command);

    private:
        // Attributes
        std::map<int, std::string> errorMap;

        // Methods
        void initErrorMap();
        void printChannelWelcome(Server &server, Client &client, Channel &channel, bool isnew);
        void partClientAll(Server &server, Client &client, std::vector<std::string> channels, std::string reason);
        void handleUserMode(Server &server, int fd, const std::vector<std::string> &tokens);
        void handleChannelMode(Server &server, int fd, const std::vector<std::string> &tokens);
        void handleUserModeShow(Server &server, int fd);
        void handleChannelModeShow(Server &server, int fd, Channel* channel);
        void updateAllModes(Server &server, int fd);

};

