/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:38:10 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/07 13:03:05 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Command.hpp"

Server::~Server()
{
	if (_socket != -1)
		close(_socket);
}

Server::Server(const std::string &port, const std::string &pass) : _port(port), _pass(pass), _socket(-1) {
}

Server::Server(const Server &src) : _port(src._port), _pass(src._pass)
{
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
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

Client &Server::getClient(int fd)
{
    if (_clients.find(fd) == _clients.end()) {
        throw std::runtime_error("Error: Client not found.");
    }
    return _clients[fd];
}

std::string Server::getClientNick(int fd) const
{
    if (_clients.find(fd) == _clients.end()) {
        throw std::runtime_error("Error: Client not found.");
    }
    return _clients.at(fd).GetNick();
}

void Server::addClientToChannel(int fd, const std::string &channel)
{
    (void)fd;
    (void)channel;
}

void Server::broadcastToChannel(const std::string &channel, const std::string &msg)
{
    (void)channel;
    (void)msg;
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

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    struct pollfd client_pollfd;
    client_pollfd.fd = client_fd;
    client_pollfd.events = POLLIN;
    _pollfds.push_back(client_pollfd);
    _clients.insert(std::make_pair(client_fd, Client(client_fd, inet_ntoa(client_addr.sin_addr))));

    std::cout << "New client connected: " << client_fd << std::endl;
    std::cout << "Client IP: " << inet_ntoa(client_addr.sin_addr) << std::endl;
    std::cout << "Client fd: " << client_fd << std::endl;
    std::cout << "Client count: " << _clients.size() << std::endl;
}

void Server::sendTimeStamps(int fd)
{
    time_t now = time(0);
    struct tm *timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "%H:%M: ", timeinfo);
    send(fd, buffer, strlen(buffer), 0);
}

void Server::checkAuth(int fd, const char *buffer)
{
    static int pass_tries = 0;
    std::string received_pass = buffer + 5;
    if (received_pass == _pass)
    {
        std::cout << "Client " << fd << " authenticated successfully." << std::endl;
        _clients[fd].SetStatus(REGISTERED);
        sendTimeStamps(fd);
        send(fd, "Password accepted.\n", 19, 0);
    }
    else
    {
        pass_tries++;
        if (pass_tries >= 3)
        {
            std::cout << "Client " << fd << " exceeded maximum password attempts." << std::endl;
            sendTimeStamps(fd);
            send(fd, "Error: Maximum password attempts exceeded. Closing connection.\n", 61, 0);
            pass_tries = 0;
            removeClient(fd);
        }
        else
        {
            std::cout << "Client " << fd << " failed to authenticate." << std::endl;
            sendTimeStamps(fd);
            send(fd, "Error: Invalid password. Please try again.\n", 43, 0);
        }
    }
    return;
}

#include "../inc/Command.hpp"

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
    std::string &clientBuffer = _clients[fd].buffer; // Store partial messages
    clientBuffer += buffer;

    size_t pos;
    while ((pos = clientBuffer.find("\r\n")) != std::string::npos) 
    {
        std::string command = clientBuffer.substr(0, pos);
        clientBuffer.erase(0, pos + 2);

        if (command.empty()) 
            continue;

        std::cout << "Received from " << fd << ": " << command << std::endl;

        try {
            Command cmd;  // Create a Command instance
            cmd.executeCommand(*this, fd, command);
        } catch (const std::exception &e) {
            std::cerr << "Command execution error: " << e.what() << std::endl;
            send(fd, "Error processing command.\r\n", 26, 0);
        }
    }
}


void Server::removeClient(int fd)
{
    std::cout << "Client disconnected: " << fd << std::endl;
    close(fd);

    for (size_t i = 0; i < _pollfds.size(); i++)
    {
        if (_pollfds[i].fd == fd)
        {
            _pollfds.erase(_pollfds.begin() + i);
            break;
        }
    }
    _clients.erase(fd);
}