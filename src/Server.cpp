/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:38:10 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/07 10:04:22 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include "../inc/Client.hpp"

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
    // sendTimeStamps(client_fd);
    // send(client_fd, "Please authenticate using PASS <password>\n", 43, 0);
    // sendTimeStamps(client_fd);
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

    // Split buffer into separate commands
    std::string data(buffer);
    size_t pos;
    while ((pos = data.find("\r\n")) != std::string::npos) 
    {
        std::string command = data.substr(0, pos);
        data.erase(0, pos + 2);

        if (command.empty()) 
            continue;

        std::cout << "Received from " << fd << ": " << command << std::endl;

        if (command.find("CAP LS") == 0)
        {
            send(fd, "CAP * LS :multi-prefix away-notify\r\n", 36, 0);
            continue;
        }

        if (command.find("CAP REQ") == 0)
        {
            send(fd, "CAP * ACK :multi-prefix away-notify\r\n", 38, 0);
            send(fd, "CAP END\r\n", 10, 0);  // End capability negotiation
            continue;
        }

        if (_clients[fd].GetStatus() != REGISTERED)
        {
            if (command.find("PASS ") == 0)
            {
                checkAuth(fd, command.c_str());
                continue;
            }

            if (!data.empty())
            {
                send(fd, "Error: You must authenticate first using PASS <password>\r\n", 58, 0);
                sendTimeStamps(fd);
                continue;
            }
        }

        if (command.find("NICK ") == 0)
        {
            std::string nickname = command.substr(5);
            _clients[fd].SetNick(nickname);
            send(fd, "Nickname set.\r\n", 15, 0);
            continue;
        }

        if (command.find("USER ") == 0)
        {
            send(fd, "User registered.\r\n", 18, 0);
            continue;
        }

        if (command.find("JOIN ") == 0)
        {
            send(fd, "Joined channel.\r\n", 17, 0);
            continue;
        }

        if (command.find("PING ") == 0)
        {
            std::string pongResponse = "PONG " + command.substr(5) + "\r\n";
            send(fd, pongResponse.c_str(), pongResponse.size(), 0);
            continue;
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