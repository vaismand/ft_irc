#include "../inc/Channel.hpp"

Channel::Channel(const int &fd, const std::string& name, const std::string& pass) : _cName(name), _cPass(pass), _cTopic(""), _isInviteOnly(false)
{
    addClient(fd);
    addOperator(fd);
}

Channel::~Channel() {}

// ----- getter Functions -----
const std::string& Channel::getcName() const { return _cName; }
const std::string& Channel::getcTopic() const { return _cTopic; }
const std::string& Channel::getcPass() const { return _cPass; }
const bool& Channel::getChannelType() const { return _isInviteOnly; }
const std::vector<int>& Channel::getJoined() const { return _joined; }

// ----- setter Functions -----
void Channel::setcName(const std::string& name) { _cName = name; }
void Channel::setcTopic(const std::string& topic) { _cTopic = topic; }
void Channel::setcPass(const std::string& password) { _cPass = password; }
void Channel::setChannelType() { _isInviteOnly = !_isInviteOnly; }

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

void Channel::addOperator(int fd) 
{
    if(!isMember(fd)) {
        dvais::sendMessage(fd, ":ircserv 442 :You're not on that channel\r\n");
        return;
    }
	for(size_t i = 0; i < _operators.size(); i++)
	{
		if (_operators[i] == fd)
			return;
	}
	_operators.push_back(fd);
}

void Channel::rmClient(int fd)
{
    for (std::vector<int>::iterator it = _joined.begin(); it != _joined.end(); )
    {
        if (*it == fd)
            it = _joined.erase(it);
        else
            ++it;
    }
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

bool Channel::isMember(int fd) const {
    std::vector<int>::const_iterator it = _joined.begin();
    std::vector<int>::const_iterator it_end = _joined.end();
    for (; it != it_end; ++it) {
        if (*it == fd)
            return true;
    }
    return false;
}

void Channel::broadcast(int fd, const std::string &message) {
    std::vector<int>::const_iterator it = _joined.begin();
    std::vector<int>::const_iterator it_end = _joined.end();

    for (; it != it_end; ++it) {
        if (fd != *it)
            dvais::sendMessage(*it, message);
    }
}
