#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include "../inc/Client.hpp"
#include "../inc/Tools.hpp"

class Channel
{
	public:
		// Constructors
		Channel(const int& fd, const std::string& name, const std::string& pass);
		~Channel();

		// Getters
		const std::string& getcName() const;
		const std::string& getcTopic() const;
		const std::string& getcKey() const;
		const bool& getChannelType() const;
		const bool& getTopicRestricted() const;
		const bool& getNoExternalMsgs() const;
		const std::vector<int>& getJoined() const;
		const size_t& getUserLimit() const;
		time_t getCreationTime() const;
		std::string getTopic() const;
        std::string getTopicSetter() const;
        std::time_t getTopicSetTime() const;
		std::string getChannelModes() const;

		// Setters
		void setcName(const std::string& name);
		void setcTopic(const std::string& topic);
		void setcKey(const std::string& key);
		void setChannelType();
        void setTopic(const std::string &topic, const std::string &setter);
		void setTopicRestricted(bool restricted);
		void setUserLimit(int limit);
		void setNoExternalMsgs(bool noExternalMsgs);
		void setModeList();

		// Methods
		void addClient(int fd);
		bool isMember(const int& fd) const;
		bool isInvited(const int& fd) const;
		void rmClient(int fd);
		void addOperator(int fd);
		bool isOperator(const int& fd) const;
		void rmOperator(int fd);
		void addInvited(int fd);
		void rmInvited(int fd);
		void broadcast(int fd, const std::string& msg);
		void clearTopic();

	private:
		// Attributes
		std::size_t 		_userLimit;
		std::string 		_cName;
		std::string 		_cKey;
		std::string 		_cTopic;
		std::string         _topicSetter;
		std::string     	_modeList;
        std::time_t         _topicSetTime;
		std::time_t     	_creationTime;
		std::vector<int> 	_joined;
		std::vector<int> 	_invited;
		std::vector<int> 	_operators;
		bool				_topicRestricted;
		bool				_isInviteOnly;
		bool				_noExternalMsgs;
};
