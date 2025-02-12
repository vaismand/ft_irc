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
    // Extract the nickname after "NICK " (assumes proper formatting).
    std::string nickname = command.substr(5);
    // Optionally: trim the nickname to remove extra spaces.
    server.getClient(fd).setNick(nickname);
    std::string msg = "Nickname set to " + nickname + "\r\n";
    dvais::sendMessage(fd, msg);
}

void Command::commandUser(Server &server, int fd, const std::string &command)
{
    // Extract the username after "USER " (assumes proper formatting).
    std::string username = command.substr(5);
    // Optionally: trim the username.
    server.getClient(fd).setUser(username);
    std::string msg = "Username set to " + username + "\r\n";
    dvais::sendMessage(fd, msg);
}

// static std::string trim(const std::string &s) {
//     size_t start = s.find_first_not_of(" \t\r\n");
//     size_t end = s.find_last_not_of(" \t\r\n");
//     if (start == std::string::npos)
//         return "";
//     return s.substr(start, end - start + 1);
// }

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
    if (server.getChannel(channelName) == NULL)
    {
        server.addChannel(channelName, "");
    }
    server.getChannel(channelName)->addClient(&server.getClient(fd));
    std::string msg = ":" + nick + " JOIN " + channelName + "\r\n";
    dvais::sendMessage(fd, msg);
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
    else
    {
        std::string nick = server.getClient(fd).getNick();
        std::string reply = ":ircserv 421 " + nick + " " + command.substr(0, command.find(" ")) + " :Unknown command\r\n";
        dvais::sendMessage(fd, reply);
    }
}
