#pragma once

#include <iostream>
#include <vector>
#include <algorithm>

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
		std::string getRealName() const;
		std::string getNick() const;
		std::string getIp() const;
		std::string& getBuffer();
		AuthState getStatus() const;
		time_t getLastActivity() const;
		int  getFd() const;
		bool getRights() const;
		bool getPingSent() const { return _pingSent; }
		bool getPassAccepted() const;
		const std::vector<std::string>& getChannelList() const;

		// Setters
		void setUser(const std::string& name);
		void setRealName(const std::string& name);
		void setNick(const std::string& name);
		void setStatus(AuthState status);
		void setRights();
		void setPassAccepted(bool passAccepted);
		void setChannelList(const std::string& channelName);
		void setPingSent(bool pingSent) { _pingSent = pingSent; }
		void setLastActivity(time_t lastActivity) { _lastActivity = lastActivity; }

		// Method
		void rmChannelInList(const std::string& channelName);
		
	private:
		// Constructors
		Client (const Client& ref);
		Client& operator= (const Client& rhs);

		// Attributes
		int _fd;
		time_t _lastActivity;
		std::string _ip;
		std::string _username;
		std::string _realname;
		std::string _nickname;
		std::string _buffer;
		std::vector<std::string> _channelList;
		AuthState	_status;
		bool		_isAdmin;
		bool 		_passAccepted;
		bool        _pingSent;

		// Methods
		void ClientInit();
};