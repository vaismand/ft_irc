/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpinchas <rpinchas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:38:34 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/06 14:31:12 by rpinchas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"

Client::Client() {}

Client::Client(int fd, const std::string& ip) : _fd(fd), _ip(ip) 
{
	ClientInit();
}


Client::Client(const Client& ref) : _fd(ref._fd), _ip(ref._ip) {}


Client::~Client()
{
}

Client& Client::operator= (const Client& rhs) {
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_ip = rhs._ip;
		_nickname = rhs._nickname;
		_username = rhs._username;
	}
	return *this;
}

void Client::ClientInit() {
	_username = "";
	_nickname = "";
	_status = UNREGISTERED;
	_isAdmin = false;
}

// ----- getter Functions -----
std::string Client::getUser() const { return _username; }
std::string Client::getNick() const { return _nickname; }
AuthState Client::getStatus() const { return _status; }
bool Client::getRights() const { return _isAdmin; }

// ----- setter Functions -----
void Client::setUser(const std::string& name) { _username = name; }
void Client::setNick(const std::string& name) { _nickname = name; }
void Client::setStatus(AuthState status) { _status = status; }
void Client::setRights() { _isAdmin = !_isAdmin; }


