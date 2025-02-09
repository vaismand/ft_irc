/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 13:37:03 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/07 13:03:01 by dvaisman         ###   ########.fr       */
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
		Client();
		Client(int fd, const std::string& ip);
		~Client();
		Client (const Client& ref);
		Client& operator= (const Client& rhs);
		
		void ClientInit();
		std::string GetUser() const;
		std::string GetNick() const;
		AuthState GetStatus() const;
		bool GetRights() const;
		void SetUser(const std::string& name);
		void SetNick(const std::string& name);
		void SetStatus(AuthState status);
		void SetRights();

		std::string buffer;
	private:

		int _fd;
		std::string _ip;
		std::string _username;
		std::string _nickname;
		AuthState	_status;
		bool		_isAdmin;
};