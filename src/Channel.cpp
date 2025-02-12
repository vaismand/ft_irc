#include "../inc/Channel.hpp"

Channel::Channel(const std::string& name, const std::string& pass) : _cName(name), _cPass(pass), _cTopic(""), _isInviteOnly(false) {}

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

void Channel::addClient(int fd) 
{ 
	for(size_t i = 0; i < _joined.size(); i++)
	{
		if (_joined[i] == fd)
			return;
	}
	_joined.push_back(fd);
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
