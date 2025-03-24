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
		std::string getUserModes() const;
		AuthState getStatus() const;
		time_t getLastActivity() const;
		int  getFd() const;
		bool getRights() const;
		bool getPingSent() const;
		bool getPassAccepted() const;
		bool isInvisible() const;
		const std::vector<std::string>& getChannelList() const;

		// Setters
		void setUser(const std::string& name);
		void setRealName(const std::string& name);
		void setNick(const std::string& name);
		void setStatus(AuthState status);
		void setRights();
		void setPassAccepted(bool passAccepted);
		void setChannelList(const std::string& channelName);
		void setPingSent(bool pingSent);
		void setInvisible(bool invisible);
		void setUserModes();
		void setLastActivity(time_t lastActivity);

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
		std::string _userModes;
		std::vector<std::string> _channelList;
		AuthState	_status;
		bool		_isAdmin;
		bool 		_passAccepted;
		bool        _pingSent;
		bool		_invisible;

		// Methods
		void ClientInit();
};