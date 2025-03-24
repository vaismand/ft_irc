#include "../inc/Server.hpp"

Server::Server(const std::string &port, const std::string &pass) : _port(port), _pass(pass), _socket(-1), _channelLimit(10)
{
}

Server::Server(const Server &src) : _port(src._port), _pass(src._pass) {}

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

// ----- getter Functions -----
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


// ----- methods -----
void Server::bindSocket()
{
	struct sockaddr_in server_addr;

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
        throw std::runtime_error("Error: Failed to create socket.");

    int opt = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("Error: Setsockopt failed.");

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
        int poll_count = poll(_pollfds.data(), _pollfds.size(), -1);
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
    _clients[client_fd] = new Client(client_fd, inet_ntoa(client_addr.sin_addr));
    std::cout << "Client Connected: " << client_fd << std::endl;
}

void Server::setPollfd(int fd, short int events, std::vector<struct pollfd> &pollfds)
{
	struct pollfd pollfd;
	pollfd.fd = fd;
	pollfd.events = events;
	pollfd.revents = 0;
	pollfds.push_back(pollfd);
}

void Server::rmClient(int fd) 
{
    std::cout << "Client disconnected: " << fd << std::endl;
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (it->second->isMember(fd))
            it->second->rmClient(fd);
    }
    close(fd);
    for (std::vector<struct pollfd>::reverse_iterator it = _pollfds.rbegin(); it != _pollfds.rend(); it++)
    {
        if (it->fd == fd)
        {
            _pollfds.erase(it.base() - 1);
            break;
        }
    }
    Client* toDelete = _clients[fd];
    _clients.erase(fd);
    delete toDelete;
}

void Server::addChannel(const int &fd, const std::string &name, const std::string &pass)
{
    _channels[name] = new Channel(fd, name, pass);
    if (bot_) {
        bot_->joinChannel(name);
    }
}

void Server::rmChannel(const std::string &name)
{
    delete _channels[name];
    _channels.erase(name);
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
    oss << ":ircserv 255 " << nick << " :There are " << _clients.size() << " clients and 1 bot service on the server" << "\r\n";
    oss << ":ircserv 375 " << nick << " :- ircserv Message of the Day -\r\n";
    oss << ":ircserv 372 " << nick << " :- Please let us pass this evaluation!\r\n";
    oss << ":ircserv 376 " << nick << " :End of /MOTD command.\r\n"; 
    dvais::sendMessage(fd, oss.str());
}

void Server::tryRegisterClient(int fd)
{
    Client &client = getClient(fd);
    
    if (client.getNick().empty() || client.getUser().empty())
        return;
    if (client.getStatus() == REGISTERED)
        return;
    if (!client.getPassAccepted())
        return;
    if (!isValidNick(client.getUser())) {
        _cmd.executeCommand(*this, fd, "NOTICE " + client.getNick() + " :*** Your username is invalid. " \
                            "Please make sure that your username contains only alphanumeric characters.\r\n");
        dvais::sendMessage(fd, "ERROR :Closing Link: Invalid username [~ " + client.getUser() + "]\r\n");
        rmClient(fd);
    }
    client.setStatus(REGISTERED);
    welcomeToServerMessage(fd, client.getNick());
}

void Server::checkIdleClients()
{
    time_t now = time(NULL);
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end();)
    {
        Client *client = it->second;
        double diff = difftime(now, client->getLastActivity());
        if (diff > 120 && !client->getPingSent())
        {
            dvais::sendMessage(client->getFd(), "PING :ircserv\r\n");
            client->setPingSent(true);
            client->setLastActivity(now);
        }
        else if (diff > 160)
        {
            int fd = client->getFd();
            std::map<int, Client*>::iterator next = it;
            ++next;
            rmClient(fd);
            it = next;
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
    clientBuffer += buffer;

    while (clientBuffer.find('\n') != std::string::npos)
    {
        std::string command = dvais::extractCommand(clientBuffer);
        if (command.empty())
            continue;
        std::cout << "Received from " << fd << ": " << command << std::endl;
        try
        {
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
    if (sockfd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::atoi(_port.c_str()));
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to connect to server" << std::endl;
        close(sockfd);
        return;
    }

    bot_ = new Bot(sockfd, "127.0.0.1", "BEEPBOOP");
    bot_->connectToServer();
}
