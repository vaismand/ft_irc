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
		
		std::string getUser() const;
		std::string getNick() const;
		std::string& getBuffer();
		AuthState getStatus() const;
		int  getFd() const;
		bool getRights() const;
		void setUser(const std::string& name);
		void setNick(const std::string& name);
		void setStatus(AuthState status);
		void setRights();
		void setPassAccepted(bool passAccepted);
		bool getPassAccepted() const;

	private:
		Client (const Client& ref);
		Client& operator= (const Client& rhs);
		int _fd;
		std::string _ip;
		std::string _username;
		std::string _nickname;
		std::string _buffer;
		AuthState	_status;
		bool		_isAdmin;
		bool 		_passAccepted;

		void ClientInit();
};