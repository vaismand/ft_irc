#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include "../inc/Client.hpp"

class Channel {
	public:
		// Constructors
		Channel(const std::string& name, const std::string& pass);
		~Channel();

		// Getters
		std::string getcName() const;
		std::string getcTopic() const;
		std::string getcPass() const;
		bool getChannelType() const;
		const std::vector<Client*>& getJoined() const;

		// Setters
		void setcName(const std::string& name);
		void setcTopic(const std::string& topic);
		void setcPass(const std::string& password);
		void setChannelType();

		// Methods
		void addClient(Client *client);
		void addOperator(Client *client);
		void rmClient(int fd);
		void rmOperator(int fd);
	private:
		std::string _cName;
		std::string _cPass;
		std::string _cTopic;
		std::vector<Client*> _joined;
		std::vector<Client*> _operators;
		bool		_isInviteOnly;
};
