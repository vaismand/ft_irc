/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:38:10 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/02 17:17:55 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

Server::~Server()
{
	if (_socket != -1)
		close(_socket);
}

Server::Server(const std::string &port, const std::string &pass) : _port(port), _pass(pass), _socket(-1)
{
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

    // Create the socket
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
        throw std::runtime_error("Error: Failed to create socket.");

    // Set socket options (SO_REUSEADDR to reuse port)
    int opt = 1;
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(std::stoi(_port));

    // Bind the socket
    if (bind(_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Error: Bind failed.");

    // Start listening
    if (listen(_socket, 10) < 0)
        throw std::runtime_error("Error: Listen failed.");

    // Add server socket to poll set
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
                if (_pollfds[i].fd == _socket)
                    addClient();
                else
                    handleClient(_pollfds[i].fd);
            }
        }
    }
}

void Server::addClient() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("accept");
        return;
    }

    // Set client socket to non-blocking
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    // Add new client to poll set
    struct pollfd client_pollfd;
    client_pollfd.fd = client_fd;
    client_pollfd.events = POLLIN;
    _pollfds.push_back(client_pollfd);

    // Store client in the map
    _clients[client_fd] = Client(client_fd);

    std::cout << "New client connected: " << client_fd << std::endl;
}

void Server::handleClient(int fd) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        removeClient(fd);
        return;
    }

    buffer[bytes_received] = '\0';
    std::cout << "Received from " << fd << ": " << buffer << std::endl;

    // Here you can process client messages based on IRC commands
}

void Server::removeClient(int fd) {
    std::cout << "Client disconnected: " << fd << std::endl;
    close(fd);

    // Remove from poll set
    for (size_t i = 0; i < _pollfds.size(); i++) {
        if (_pollfds[i].fd == fd) {
            _pollfds.erase(_pollfds.begin() + i);
            break;
        }
    }

    // Remove from client map
    _clients.erase(fd);
}