#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include "../inc/Client.hpp"
#include "../inc/Tools.hpp"

class Channel {
	public:
		// Constructors
		Channel(const int& fd, const std::string& name, const std::string& pass);
		~Channel();

		// Getters
		const std::string& getcName() const;
		const std::string& getcTopic() const;
		const std::string& getcPass() const;
		const bool& getChannelType() const;
		const std::vector<int>& getJoined() const;
		std::string getTopic() const;
        std::string getTopicSetter() const;
        std::time_t getTopicSetTime() const;


		// Setters
		void setcName(const std::string& name);
		void setcTopic(const std::string& topic);
		void setcPass(const std::string& password);
		void setChannelType();
        void setTopic(const std::string &topic, const std::string &setter);


		// Methods
		void addClient(int fd);
		bool isMember(int fd) const;
		void rmClient(int fd);
		void addOperator(int fd);
		bool isOperator(const int& fd) const;
		void rmOperator(int fd);
		void broadcast(int fd, const std::string& msg);
		void clearTopic();


	private:
		// Attributes
		int 				_userLimit;
		std::string 		_cName;
		std::string 		_cPass;
		std::string 		_cTopic;
		std::string         _topicSetter;
        std::time_t         _topicSetTime;
		std::vector<int> 	_joined;
		std::vector<int> 	_operators;
		bool				_topicRestricted;
		bool				_isInviteOnly;
};
