/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:38:34 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/06 14:06:56 by dvaisman         ###   ########.fr       */
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

// ----- Getter Functions -----
std::string Client::GetUser() const { return _username; }
std::string Client::GetNick() const { return _nickname; }
AuthState Client::GetStatus() const { return _status; }
bool Client::GetRights() const { return _isAdmin; }

// ----- Setter Functions -----
void Client::SetUser(const std::string& name) { _username = name; }
void Client::SetNick(const std::string& name) { _nickname = name; }
void Client::SetStatus(AuthState status) { _status = status; }
void Client::SetRights() { _isAdmin = !_isAdmin; }


