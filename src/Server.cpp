#include "../inc/Server.hpp"

// ----- Constructors -----

Server::Server() {}

Server::Server(const std::string &port, const std::string &pass) :_port(port), _pass(pass), _socket(-1), _channelLimit(10), bot_(NULL)
{
    signal(SIGPIPE, SIG_IGN);
}

Server::~Server()
{
	if (_socket != -1)
		close(_socket);
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        delete it->second;
    }
    _clients.clear();
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        delete it->second;
    }
    _channels.clear();
    for (std::vector<struct pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); it++)
    {
        close(it->fd);
    }
    _pollfds.clear();
}

Server::Server(const Server &obj) : _port(obj._port), _pass(obj._pass), _socket(obj._socket), _channelLimit(obj._channelLimit)
{
    *this = obj;
}

Server &Server::operator=(const Server &rhs)
{
    if (this != &rhs)
    {
        _clients = rhs._clients;
        _channels = rhs._channels;
        _pollfds = rhs._pollfds;
        bot_ = rhs.bot_;
    }
    return *this;
}

// ----- Getter Functions -----
std::string Server::getPass() const { return _pass; }

const std::map<int, Client*>& Server::getClients() const { return _clients; }

std::size_t Server::getChannelLimit() const { return _channelLimit; }

Client &Server::getClient(int fd) const
{
    if (_clients.find(fd) == _clients.end())
        throw std::runtime_error("Error: Client not found.");
    return *_clients.at(fd);
}

std::string Server::getClientNick(int fd) const
{
    if (_clients.find(fd) == _clients.end())
        throw std::runtime_error("Error: Client not found.");
    return _clients.at(fd)->getNick();
}

Client* Server::getClientByNick(const std::string &nick)
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNick() == nick)
            return it->second;
    }
    return NULL;
}

Channel *Server::getChannel(const std::string &name)
{
    if (_channels.find(name) == _channels.end())
        return NULL;
    return _channels[name];
}

bool Server::shareChannel(int fd1, int fd2) const
{
    for (std::map<std::string, Channel*>::const_iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        const Channel* ch = it->second;
        if (ch->isMember(fd1) && ch->isMember(fd2))
            return true;
    }
    return false;
}

Bot &Server::getBot() const
{
    return *bot_;
}

// ----- Methods -----
void Server::bindSocket()
{
	struct sockaddr_in server_addr;

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    { 
        std::ostringstream oss; 
        oss << "Error in bindSocket: Failed to create socket. (" << strerror(errno) << ")"; 
        throw std::runtime_error(oss.str());
    }

    int opt = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::ostringstream oss;
        oss << "Error in bindSocket: setsockopt failed. (" << strerror(errno) << ")";
        throw std::runtime_error(oss.str());
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(std::atoi(_port.c_str()));

    if (bind(_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Error: Bind failed.");
    if (listen(_socket, 10) < 0)
        throw std::runtime_error("Error: Listen failed.");

    setPollfd(_socket, POLLIN, _pollfds);
    std::cout << "Server started, listening on port " << _port << std::endl;
}

void Server::run()
{
    bindSocket();
    createBot();
    while (g_running)
    {
        int poll_count = poll(_pollfds.data(), _pollfds.size(), 1000);
        if (poll_count < 0)
        {
            if (errno == EINTR)
            {
                if (!g_running)
                break;
                continue;
            }
            perror("poll");
            break;
        }
        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                rmClient(_pollfds[i].fd);
                continue;
            }
            if (_pollfds[i].revents & POLLIN)
            {
                if (_pollfds[i].fd == _socket) {
                    addClient();
                }
                else {
                    handleClient(_pollfds[i].fd);
                }
            }
        }
        if (bot_) {
            bot_->sendRandomPhrase();
        }
        checkIdleClients();
    }
    std::cerr << "Server shutting down..." << std::endl;
}

bool Server::isNickInUse(const std::string &nickname, int excludeFd) const {
    for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->first != excludeFd && it->second->getNick() == nickname) {
            return true;
        }
    }
    return false;
}

bool Server::isValidNick(const std::string &nickname) {
    if (nickname.empty() || !isalpha(nickname[0]))
        return false;
    for (size_t i = 0; i < nickname.length(); ++i) {
        char c = nickname[i];
        if (!isalnum(c) && c != '-' && c != '_')
            return false;
    }
    return true;
}

void Server::addClient()
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(_socket, (struct sockaddr*)&client_addr, &addr_len);
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    
    if (client_fd < 0)
    {
        perror("accept");
        return;
    }
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl");
        close(client_fd);
        return;
    }
    setPollfd(client_fd, POLLIN, _pollfds);
    
    if (!bot_) {
        bot_ = new Bot(client_fd, inet_ntoa(client_addr.sin_addr), "BEEPBOOP");
        _clients[client_fd] = bot_;
        bot_->connectToServer(_pass);
        std::cout << "Bot connected successfully: FD = " << client_fd << std::endl;
        bot_->setConnected(true);
    } else {
        Client* newClient = new Client(client_fd, inet_ntoa(client_addr.sin_addr));
        _clients[client_fd] = newClient;
        std::cout << "New client connected: FD = " << client_fd << std::endl;
    }
}

void Server::rmClient(int fd) 
{
    std::cout << "Client disconnected: " << fd << std::endl;
    Client* clientToDel = _clients[fd];
    const std::vector<std::string>& channelList = clientToDel->getChannelList();

    for (std::vector<std::string>::const_iterator it = channelList.begin(); it != channelList.end(); ++it) {
        Channel* channel = getChannel(*it);
        if (!channel) {
            continue;
        }
        if (!clientToDel->getHasQuit()) {
        std::string quitMsg = ":" + clientToDel->getNick() + "!~" + clientToDel->getUser()
                        + "@" + clientToDel->getIp() + " QUIT :Client disconnected\r\n";
        channel->broadcast(fd, quitMsg);
        }
        channel->rmClientFromChannel(fd);
        if (isChannelEmptyOrBotOnly(channel)) {
            handleEmptyChannel(channel);
        }
    }
    close(fd);
    rmPollfd(fd);
    _clients.erase(fd);
    delete clientToDel;
}

void Server::addChannel(const int &fd, const std::string &name, const std::string &pass)
{
    _channels[name] = new Channel(fd, name, pass);
    if (bot_) {
        bot_->joinChannel(name);
    }
}

void Server::rmChannel(const std::string &name) {
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end()) {
        delete it->second;
        _channels.erase(it);
    }
}

void Server::broadcastAll(int fd, const std::string &msg)
{
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (it->second->isMember(fd))
            it->second->broadcast(fd, msg);
    }
}

void Server::welcomeToServerMessage(int fd, const std::string &nick)
{
    std::ostringstream oss;
    std::string timeStamp = std::string(BUILD_DATE) + " " + BUILD_TIME;
    oss << ":ircserv 001 " << nick << " :Welcome to the IRC server " << nick << "!\r\n";
    oss << ":ircserv 002 " << nick << " :Your host is ircserv, running version 69.0\r\n";
    oss << ":ircserv 003 " << nick << " :This server was compiled " << timeStamp << "\r\n";
    oss << ":ircserv 004 " << nick << " ircserv 69.0 channel modes:bitklon\r\n";
    oss << ":ircserv 005 " << nick << " CHANTYPES=# CHANMODES=bitklon CHANLIMIT=10 PREFIX=@ " << \
    "NICKLEN=16 USERLEN=16 CHANNELLEN=20 TOPICLEN=200 are supported by this server" << "\r\n";
    oss << ":ircserv 005 " << nick << " TARGMAX=NAMES:1,LIST:1,KICK:1,WHOIS:1,PRIVMSG:1,NOTICE:1 are supported by this server\r\n";
    oss << ":ircserv 254 " << nick << " :There are " << _channels.size() << " channels on the server" << "\r\n";
    oss << ":ircserv 255 " << nick << " :There are " << _clients.size() - 1 << " clients and 1 bot service on the server" << "\r\n";
    oss << ":ircserv 375 " << nick << " :- ircserv Message of the Day -\r\n";
    oss << ":ircserv 372 " << nick << " :- Please let us pass this evaluation!\r\n";
    oss << ":ircserv 376 " << nick << " :End of /MOTD command.\r\n"; 
    dvais::sendMessage(fd, oss.str());
}

void Server::tryRegisterClient(int fd)
{
    Client &client = getClient(fd);
    
    if (!client.getPassAccepted())
        return;
    if (client.getNick().empty() || client.getUser().empty() || client.getNick() == "*")
        return;
    if (client.getStatus() == REGISTERED)
        return;
    client.setStatus(REGISTERED);
    welcomeToServerMessage(fd, client.getNick());
}

void Server::checkIdleClients()
{
    const time_t now = time(NULL);
    const int pingInterval = 180;
    const int pingTimeout  = 60; 

    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end();)
    {
        Client *client = it->second;
        int fd = it->first;

        if (bot_ && fd == bot_->getFd())
        {
            ++it;
            continue;
        }
        if (!client->getStatus()) {
            ++it;
            continue;
        }

        double idleTime = difftime(now, client->getLastActivity());

        if (idleTime >= pingInterval && !client->getPingSent())
        {
            dvais::sendMessage(client->getFd(), "PING :ircserv\r\n");
            client->setPingSent(true);
            client->setPingSentTime(now);
            ++it;
        }
        else if (client->getPingSent() && difftime(now, client->getPingSentTime()) >= pingTimeout)
        {
            int fd = client->getFd();
            std::cout << "Client " << client->getNick() << " timed out. Disconnecting.\n";
            ++it;
            rmClient(fd);    
        }
        else
        {
            ++it;
        }
    }
}

void Server::handleClient(int fd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        rmClient(fd);
        return;
    }
    _clients[fd]->setLastActivity(time(NULL));
    buffer[bytes_received] = '\0';
    std::string &clientBuffer = _clients[fd]->getBuffer();
    if (clientBuffer.length() + bytes_received > 512)
    {
        std::cerr << "Client sent oversized message, ignoring message." << std::endl;
        dvais::sendMessage(fd, ":ircserv 417 * :Message too long (limit is 512 chars)\r\n");
        clientBuffer.clear();
        return;
    }
    clientBuffer += buffer;

    while (clientBuffer.find('\n') != std::string::npos)
    {
        std::string command = dvais::extractCommand(clientBuffer);
        if (command.empty())
            continue;
        std::cout << "Received from socket " << fd << ": " << command << std::endl;
        try
        {
            Client &client = getClient(fd);
            if (!client.getPassAccepted()) 
            {
                if (command.find("PASS ") != 0 && command.find("CAP ") != 0) {
                    _cmd.sendError(fd, 998, "", "");
                    continue;
                }
            }
            _cmd.executeCommand(*this, fd, command);
        } 
        catch (const std::exception &e)
        {
            std::cerr << "Command execution error: " << e.what() << std::endl;
            if (dvais::sendMessage(fd, "Error processing command.\r\n") < 0)
            {
                rmClient(fd);
                return;
            }
        }
        if (_clients.find(fd) == _clients.end())
            return;
        tryRegisterClient(fd);
    }
}

void Server::createBot() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) return;

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::atoi(_port.c_str()));
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        close(sockfd);
        return;
    }
}
