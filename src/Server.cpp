#include "../inc/Server.hpp"

Server::Server(const std::string &port, const std::string &pass) : _port(port), _pass(pass), _socket(-1) 
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
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        delete it->second;
    }
    _clients.clear();
}

std::string Server::getPass() const
{
	return _pass;
}

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

    struct pollfd server_pollfd;
    server_pollfd.fd = _socket;
    server_pollfd.events = POLLIN; 
    server_pollfd.revents = 0;
    _pollfds.push_back(server_pollfd);

    std::cout << "Server started, listening on port " << _port << std::endl;
}

void Server::run()
{
    bindSocket();
    while (true)
	{
        int poll_count = poll(_pollfds.data(), _pollfds.size(), -1);
        if (poll_count < 0)
		{
            perror("poll");
            break;
        }
        for (size_t i = 0; i < _pollfds.size(); i++)
		{
            if (_pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                removeClient(_pollfds[i].fd);
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
    }
}

Client &Server::getClient(int fd) const
{
    if (_clients.find(fd) == _clients.end())
    {
        throw std::runtime_error("Error: Client not found.");
    }
    return *_clients.at(fd);
}

std::string Server::getClientNick(int fd) const
{
    if (_clients.find(fd) == _clients.end()) {
        throw std::runtime_error("Error: Client not found.");
    }
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
    struct pollfd client_pollfd;
    client_pollfd.fd = client_fd;
    client_pollfd.events = POLLIN;
    client_pollfd.revents = 0;
    _pollfds.push_back(client_pollfd);
    _clients[client_fd] = new Client(client_fd, inet_ntoa(client_addr.sin_addr));
    std::cout << "Client Connected: " << client_fd << std::endl;
}

void Server::removeClient(int fd) 
{
    std::cout << "Client disconnected: " << fd << std::endl;
    close(fd);

    for (std::vector<struct pollfd>::reverse_iterator it = _pollfds.rbegin(); it != _pollfds.rend(); it++)
    {
        if (it->fd == fd)
        {
            _pollfds.erase(it.base() - 1);
            break;
        }
    }
    _clients.erase(fd);
}

Channel *Server::getChannel(const std::string &name)
{
    if (_channels.find(name) == _channels.end())
        return NULL;
    return _channels[name];
}

void Server::addChannel(const int &fd, const std::string &name, const std::string &pass)
{
    _channels[name] = new Channel(fd, name, pass);
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

void Server::tryRegisterClient(int fd)
{
    Client &client = getClient(fd);
    
    if (client.getNick().empty() || client.getUser().empty())
        return;
    if (client.getStatus() == REGISTERED)
        return;
    if (!client.getPassAccepted())
        return;
    client.setStatus(REGISTERED);

    std::string welcome = ":ircserv 001 " + client.getNick() + " :Welcome to the IRC server " + client.getNick() + "!\r\n";
    dvais::sendMessage(fd, welcome);
}

void Server::handleClient(int fd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        removeClient(fd);
        return;
    }

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
            Command cmd;
            cmd.executeCommand(*this, fd, command);
        } 
        catch (const std::exception &e)
        {
            std::cerr << "Command execution error: " << e.what() << std::endl;
            if (dvais::sendMessage(fd, "Error processing command.\r\n") < 0)
            {
                removeClient(fd);
                return;
            }
        }
        if (_clients.find(fd) == _clients.end())
            return;
        tryRegisterClient(fd);
    }
}

bool Server::isNickInUse(const std::string &nickname, int excludeFd) const {
    for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->first != excludeFd && it->second->getNick() == nickname) {
            return true;
        }
    }
    return false;
}

const std::map<int, Client*>& Server::getClients() const
{
    return _clients;
}
