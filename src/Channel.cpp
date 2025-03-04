#include "../inc/Channel.hpp"

Channel::Channel(const int &fd, const std::string& name, const std::string& key) : _cName(name), _cKey(key), _cTopic(""), _isInviteOnly(false)
{
    addClient(fd);
    _operators.push_back(fd);
}

Channel::~Channel() {}

// ----- getter Functions -----
const std::string& Channel::getcName() const { return _cName; }
const std::string& Channel::getcTopic() const { return _cTopic; }
const std::string& Channel::getcKey() const { return _cKey; }
const bool& Channel::getChannelType() const { return _isInviteOnly; }
const bool& Channel::getTopicRestricted() const { return _topicRestricted; }
const std::vector<int>& Channel::getJoined() const { return _joined; }
const int& Channel::getUserLimit() const { return _userLimit; }
std::string Channel::getTopic() const { return _cTopic; }
std::string Channel::getTopicSetter() const { return _topicSetter; }
std::time_t Channel::getTopicSetTime() const { return _topicSetTime; }

// ----- setter Functions -----
void Channel::setcName(const std::string& name) { _cName = name; }
void Channel::setcTopic(const std::string& topic) { _cTopic = topic; }
void Channel::setcKey(const std::string& key) { _cKey = key; }
void Channel::setChannelType() { _isInviteOnly = !_isInviteOnly; }
void Channel::setTopicRestricted(bool restricted) { _topicRestricted = restricted; }
void Channel::setUserLimit(int limit) { _userLimit = limit; }

// ----- methods -----
void Channel::addClient(int fd) 
{   
    if (_joined.empty()) {
	    _joined.push_back(fd);
        return;
    }
	for(size_t i = 0; i < _joined.size(); i++)
	{
		if (_joined[i] == fd)
			return;
	}
	_joined.push_back(fd);
}

bool Channel::isMember(const int &fd) const {
    return std::find(_joined.begin(), _joined.end(), fd) != _joined.end();

}

void Channel::rmClient(int fd)
{
    for (std::vector<int>::iterator it = _joined.begin(); it != _joined.end(); )
    {
        if (*it == fd) {
            it = _joined.erase(it);
        } else {
            ++it;
        }
    }
}

void Channel::addOperator(int fd) 
{
    if(!isMember(fd)) {
        dvais::sendMessage(fd, ":ircserv 442 :You're not on that channel\r\n"); // handle error message correctly
        return;
    }
    if(!isOperator(fd)) {
        dvais::sendMessage(fd, ":ircserv 442 :You have not the rights\r\n"); // handle error message correctly
        return;
    }
	for(size_t i = 0; i < _operators.size(); i++)
	{
		if (_operators[i] == fd)
			return;
	}
	_operators.push_back(fd);
}

bool Channel::isOperator(const int &fd) const {
    return std::find(_operators.begin(), _operators.end(), fd) != _operators.end();
}

void Channel::rmOperator(int fd) {
    for (std::vector<int>::iterator it = _operators.begin(); it != _operators.end(); )
    {
        if (*it == fd)
            it = _operators.erase(it);
        else
            ++it;
    }
}

void Channel::addInvited(int fd) 
{
    if (!isMember(fd) && !isInvited(fd))
        _invited.push_back(fd);
}

bool Channel::isInvited(const int &fd) const {
    return std::find(_invited.begin(), _invited.end(), fd) != _invited.end();
}

void Channel::broadcast(int fd, const std::string &message) {
    std::vector<int>::const_iterator it = _joined.begin();
    std::vector<int>::const_iterator it_end = _joined.end();

    for (; it != it_end; ++it) {
        if (fd != *it)
            dvais::sendMessage(*it, message);
    }
}

void Channel::setTopic(const std::string &topic, const std::string &setter) {
    _cTopic = topic;
    _topicSetter = setter;
    _topicSetTime = std::time(NULL);
}

void Channel::clearTopic() {
    _cTopic.clear();
    _topicSetter.clear();
    _topicSetTime = 0;
}

