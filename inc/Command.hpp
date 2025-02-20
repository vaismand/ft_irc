#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <map>
#include <vector>
#include "Server.hpp"

class Server;

class Command {
public:
    Command();
    Command(const Command &src);
    Command &operator=(const Command &src);
    ~Command();

    void commandCap(int fd, const std::string &command);
    void commandNick(Server &server, int fd, const std::string &command);
    void commandUser(Server &server, int fd, const std::string &command);
    void commandJoin(Server &server, int fd, const std::string &command);
    void commandPart(Server &server, int fd, const std::string &command);
    void commandMode(Server &server, int fd, const std::string &command);
    void commandPing(int fd, const std::string &command);
    void commandPass(Server &server, int fd, const std::string &command);
    void commandPrivmsg(Server &server, int fd, const std::string &command);
    void executeCommand(Server &server, int fd, const std::string &command);

private:
    std::map<int, std::string> errorMap;
    std::string getErrorMessage(int errorCode, const std::string &nick, const std::string &command = "");
    std::string trim(const std::string &s);
    bool isValidNick(const std::string &nickname);
    void initErrorMap();
};

#endif // COMMAND_HPP