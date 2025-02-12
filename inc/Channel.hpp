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
		const std::string& getcName() const;
		const std::string& getcTopic() const;
		const std::string& getcPass() const;
		const bool& getChannelType() const;
		const std::vector<int>& getJoined() const;

		// Setters
		void setcName(const std::string& name);
		void setcTopic(const std::string& topic);
		void setcPass(const std::string& password);
		void setChannelType();

		// Methods
		void addClient(int fd);
		void addOperator(int fd);
		void rmClient(int fd);
		void rmOperator(int fd);
	private:
		std::string _cName;
		std::string _cPass;
		std::string _cTopic;
		std::vector<int> _joined;
		std::vector<int> _operators;
		bool		_isInviteOnly;
};
