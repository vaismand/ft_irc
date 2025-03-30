#include "../inc/Channel.hpp"

Channel::Channel() {}

Channel::Channel(const int &fd, const std::string& name, const std::string& key) : _cName(name), 
_cKey(key), _cTopic("")
{
    addClientToChannel(fd);
    _operators.push_back(fd);
    _userLimit = 0;
    _noExternalMsgs = true;
    _isInviteOnly = false;
    _topicRestricted = true;
    _modeList = "";
    _creationTime = std::time(NULL);
    _topicSetTime = 0;
    _topicSetter = "";
}

Channel::~Channel() {
    _joined.clear();
    _invited.clear();
    _operators.clear();
}

Channel::Channel(const Channel& obj)
{
    _userLimit = obj._userLimit;
    _cName = obj._cName;
    _cKey = obj._cKey;
    _cTopic = obj._cTopic;
    _topicSetter = obj._topicSetter;
    _modeList = obj._modeList;
    _topicSetTime = obj._topicSetTime;
    _creationTime = obj._creationTime;
    _joined = obj._joined;
    _invited = obj._invited;
    _operators = obj._operators;
    _topicRestricted = obj._topicRestricted;
    _isInviteOnly = obj._isInviteOnly;
    _noExternalMsgs = obj._noExternalMsgs;
}
Channel& Channel::operator=(const Channel& rhs)
{
    if (this != &rhs) {
        _userLimit = rhs._userLimit;
        _cName = rhs._cName;
        _cKey = rhs._cKey;
        _cTopic = rhs._cTopic;
        _topicSetter = rhs._topicSetter;
        _modeList = rhs._modeList;
        _topicSetTime = rhs._topicSetTime;
        _creationTime = rhs._creationTime;
        _joined = rhs._joined;
        _invited = rhs._invited;
        _operators = rhs._operators;
        _topicRestricted = rhs._topicRestricted;
        _isInviteOnly = rhs._isInviteOnly;
        _noExternalMsgs = rhs._noExternalMsgs;
    }
    return *this;
}

// ----- getter Functions -----
const std::string& Channel::getcName() const { return _cName; }
const std::string& Channel::getcTopic() const { return _cTopic; }
const std::string& Channel::getcKey() const { return _cKey; }
bool Channel::getChannelType() const { return _isInviteOnly; }
bool Channel::getTopicRestricted() const { return _topicRestricted; }
bool Channel::getNoExternalMsgs() const { return _noExternalMsgs; }
const std::vector<int>& Channel::getJoined() const { return _joined; }
size_t Channel::getUserLimit() const { return _userLimit; }
time_t Channel::getCreationTime() const { return _creationTime; }
std::string Channel::getTopicSetter() const { return _topicSetter; }
std::string Channel::getChannelModes() const { return _modeList; }
std::time_t Channel::getTopicSetTime() const { return _topicSetTime; }

// ----- setter Functions -----
void Channel::setcName(const std::string& name) { _cName = name; }
void Channel::setcTopic(const std::string& topic) { _cTopic = topic; }
void Channel::setcKey(const std::string& key) { _cKey = key; }
void Channel::setChannelType(bool inviteOnly) { _isInviteOnly = inviteOnly; }
void Channel::setTopicRestricted(bool restricted) { _topicRestricted = restricted; }
void Channel::setUserLimit(int limit) { _userLimit = limit; }
void Channel::setNoExternalMsgs(bool noExternalMsgs) { _noExternalMsgs = noExternalMsgs; }

void Channel::setModeList() {
    _modeList.clear();
    _modeList = "+";
    if (_isInviteOnly)
        _modeList += "i";
    if (_noExternalMsgs)
        _modeList += "n";
    if (_topicRestricted)
        _modeList += "t";
    if (!_cKey.empty())
        _modeList += "k";
    if (_userLimit > 0)
        _modeList += "l";
}

// ----- methods -----
void Channel::addClientToChannel(int fd) 
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

void Channel::rmClientFromChannel(int fd)
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
        dvais::sendMessage(fd, ":ircserv 482 :You're not channel operator\r\n"); // handle error message correctly
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

void Channel::rmInvited(int fd) {
    for (std::vector<int>::iterator it = _invited.begin(); it != _invited.end(); )
    {
        if (*it == fd)
            it = _invited.erase(it);
        else
            ++it;
    }
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

