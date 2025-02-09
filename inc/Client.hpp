/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:37:03 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/09 19:14:29 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

enum AuthState {
	UNREGISTERED,
	REGISTERED
};

class Client
{
	public:
		Client(int fd, const std::string& ip);
		~Client();
		Client (const Client& ref);
		Client& operator= (const Client& rhs);
		
		std::string getUser() const;
		std::string getNick() const;
		AuthState getStatus() const;
		bool getRights() const;
		void setUser(const std::string& name);
		void setNick(const std::string& name);
		void setStatus(AuthState status);
		void setRights();

		std::string buffer;
	private:
		int _fd;
		std::string _ip;
		std::string _username;
		std::string _nickname;
		AuthState	_status;
		bool		_isAdmin;

		void ClientInit();
};