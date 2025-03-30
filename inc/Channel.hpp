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
		const std::vector<int>& getJoined() const;
		bool getTopicRestricted() const;
		bool getNoExternalMsgs() const;
		bool getChannelType() const;
		size_t getUserLimit() const;
		time_t getCreationTime() const;
        std::string getTopicSetter() const;
        std::time_t getTopicSetTime() const;
		std::string getChannelModes() const;

		// Setters
		void setcName(const std::string& name);
		void setcTopic(const std::string& topic);
		void setcKey(const std::string& key);
		void setChannelType(bool inviteOnly);
        void setTopic(const std::string &topic, const std::string &setter);
		void setTopicRestricted(bool restricted);
		void setUserLimit(int limit);
		void setNoExternalMsgs(bool noExternalMsgs);
		void setModeList();

		// Methods
		void addClientToChannel(int fd);
		bool isMember(const int& fd) const;
		bool isInvited(const int& fd) const;
		void rmClientFromChannel(int fd);
		void addOperator(int fd);
		bool isOperator(const int& fd) const;
		void rmOperator(int fd);
		void addInvited(int fd);
		void rmInvited(int fd);
		void broadcast(int fd, const std::string& msg);
		void clearTopic();

	private:
		// Constructors
		Channel();
		Channel(const Channel& obj);
		Channel& operator=(const Channel& rhs);
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
