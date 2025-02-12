#pragma once

#include <iostream>

enum AuthState {
	UNREGISTERED,
	REGISTERED
};

class Client
{
	public:
		// Constructors
		Client(int fd, const std::string& ip);
		~Client();
		
		// Getters
		std::string getUser() const;
		std::string getNick() const;
		std::string& getBuffer();
		AuthState getStatus() const;
		int  getFd() const;
		bool getRights() const;

		// Setters
		void setUser(const std::string& name);
		void setNick(const std::string& name);
		void setStatus(AuthState status);
		void setRights();
		void setPassAccepted(bool passAccepted);
		bool getPassAccepted() const;

	private:
		// Constructors
		Client (const Client& ref);
		Client& operator= (const Client& rhs);

		// Attributes
		int _fd;
		std::string _ip;
		std::string _username;
		std::string _nickname;
		std::string _buffer;
		AuthState	_status;
		bool		_isAdmin;
		bool 		_passAccepted;

		// Methods
		void ClientInit();
};