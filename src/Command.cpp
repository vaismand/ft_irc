#include "../inc/Command.hpp"
#include <map>
#include <sstream>
#include <vector>
#include <cstdlib>

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
    errorMap[421] = "Unknown command";
    errorMap[432] = "Erroneous nickname";
    errorMap[451] = "You have not registered";
    errorMap[461] = "Not enough parameters";
    errorMap[464] = "Password incorrect";
    errorMap[475] = "Cannot join channel (invite only)";
    errorMap[403] = "No such channel";
}

std::string Command::getErrorMessage(int errorCode, const std::string &nick, const std::string &command)
{
    std::ostringstream oss;
    oss << ":ircserv " << errorCode << " " << nick << " ";
    if (!command.empty()) {
        oss << command << " ";
    }
    oss << ":" << errorMap[errorCode] << "\r\n";
    return oss.str();
}

std::string Command::trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
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
    dvais::sendMessage(fd, "Error: Unknown CAP subcommand.\r\n");
}

void Command::commandNick(Server &server, int fd, const std::string &command) {
    std::string nickname = command.substr(5);
    nickname = trim(nickname);
    if (nickname.empty() || nickname.length() > 9 || !isValidNick(nickname)) {
        std::string reply = getErrorMessage(432, "", nickname);
        dvais::sendMessage(fd, reply);
        return;
    }

    server.getClient(fd).setNick(nickname);
    std::string msg = "Nickname set to " + nickname + "\r\n";
    dvais::sendMessage(fd, msg);
}

void Command::commandUser(Server &server, int fd, const std::string &command) {
    std::string username = command.substr(5);
    username = trim(username);
    server.getClient(fd).setUser(username);
    std::string msg = "Username set to " + username + "\r\n";
    dvais::sendMessage(fd, msg);
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
    std::string channelName = command.substr(5);

    if (channelName.find(",") != std::string::npos) {
        std::string reply = getErrorMessage(475, nick, channelName);
        dvais::sendMessage(fd, reply);
        return;
    }
    if (channelName.find(" ") != std::string::npos) {
        std::string reply = getErrorMessage(461, nick, "JOIN");
        dvais::sendMessage(fd, reply);
        return;
    }
    if (channelName.find("#") != 0) {
        std::string reply = getErrorMessage(403, nick, channelName);
        dvais::sendMessage(fd, reply);
        return;
    }
    if (server.getClient(fd).getStatus() != REGISTERED) {
        std::string reply = getErrorMessage(451, nick, "JOIN");
        dvais::sendMessage(fd, reply);
        return;
    }
    if (server.getClient(fd).getNick().empty()) {
        std::string reply = getErrorMessage(451, nick, "JOIN");
        dvais::sendMessage(fd, reply);
        return;
    }
    if (server.getClient(fd).getUser().empty()) {
        std::string reply = getErrorMessage(451, nick, "JOIN");
        dvais::sendMessage(fd, reply);
        return;
    }
    if (!server.getClient(fd).getPassAccepted()) {
        std::string reply = getErrorMessage(464, nick, "JOIN");
        dvais::sendMessage(fd, reply);
        return;
    }
    Channel* ChannelToJoin = server.getChannel(channelName);
    if (ChannelToJoin == NULL) {
        server.addChannel(channelName, "");
        ChannelToJoin = server.getChannel(channelName);
    }
    ChannelToJoin->addClient(server.getClient(fd).getFd());
    std::string msg = ":" + nick + " JOIN " + channelName + "\r\n";
    ChannelToJoin->broadcast(-1, msg);
}

void Command::commandPart(Server &server, int fd, const std::string &command) {
    std::string nick = server.getClient(fd).getNick();
    std::string channelName = command.substr(5);
    Channel* tmp = server.getChannel(channelName);
    if (!tmp) {
        std::string reply = getErrorMessage(421, nick, channelName);
        dvais::sendMessage(fd, reply);
        return;
    }
    if (!tmp->isMember(fd)) {
        std::string reply = getErrorMessage(442, nick, channelName);
        dvais::sendMessage(fd, reply);
        return;
    }
    std::string msg = ":" + nick + " PART " + channelName + "\r\n";
    tmp->broadcast(fd, msg);
    tmp->rmClient(fd);
}

void Command::commandMode(Server &server, int fd, const std::string &command) {
    (void)command;
    std::string nick = server.getClient(fd).getNick();
    std::string reply = getErrorMessage(421, nick, "MODE");
    dvais::sendMessage(fd, reply);
}

void Command::commandPing(int fd, const std::string &command) {
    std::string servername = command.substr(5);
    std::string msg = "PONG " + servername + "\r\n";
    dvais::sendMessage(fd, msg);
}

void Command::commandPass(Server &server, int fd, const std::string &command) {
    std::string providedPass = command.substr(5);
    if (providedPass != server.getPass()) {
        std::string reply = getErrorMessage(464, "", "");
        dvais::sendMessage(fd, reply);
        server.removeClient(fd);
    } else {
        server.getClient(fd).setPassAccepted(true);
    }
}

static std::vector<std::string> cmdtokenizer(const std::string& command) {
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;

    if (command.empty())
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

void Command::commandPrivmsg(Server &server, int fd, const std::string &command) {
    std::vector<std::string> cmd = cmdtokenizer(command);
    if (cmd.empty()) {
        std::cout << "place error msg here" << std::endl; // error handling needs to be solved.
        return;
    }
    // Check if Channel has a #, if Channel exists and if Client is in Channel and if it has the needed rights.
    std::size_t chanPos = cmd[1].find("#");
    if (chanPos != 0) {
        std::cout << "place error msg here" << std::endl; // error handling needs to be solved.
        return;
    }
    Channel* ChannelToChat = server.getChannel(cmd[1]);
    if (ChannelToChat == NULL) {
        std::cout << "place error msg here" << std::endl; // error handling needs to be solved.
        return;
    }
    if (!ChannelToChat->isMember(fd)) {
        dvais::sendMessage(fd, ":ircserv 442 " + ChannelToChat->getcName() + ": You're not on that channel\r\n");
        return;
    }
    // Create the message to be broadcasted to all members of the channel.
    std::size_t msgPos = cmd[2].find_first_of(":");
    if (msgPos != 0) {
        std::cout << "place error msg here" << std::endl; // error handling needs to be solved.
        return;
    }
    std::string msg = ":" + server.getClient(fd).getNick() + " " + cmd[0] + " " + ChannelToChat->getcName() + " " + cmd[2] + " \r\n";
    ChannelToChat->broadcast(fd, msg);
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
    } else {
        std::string nick = server.getClient(fd).getNick();
        std::string reply = getErrorMessage(421, nick, command.substr(0, command.find(" ")));
        dvais::sendMessage(fd, reply);
    }
}