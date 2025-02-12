#include "../inc/Channel.hpp"

Channel::Channel(const std::string& name, const std::string& pass) : _cName(name), _cPass(pass), _cTopic(""), _isInviteOnly(false) {}

Channel::~Channel() {}

// ----- getter Functions -----
std::string Channel::getcName() const { return _cName; }
std::string Channel::getcTopic() const { return _cTopic; }
std::string Channel::getcPass() const { return _cPass; }
bool Channel::getChannelType() const { return _isInviteOnly; }
const std::vector<Client*>& Channel::getJoined() const { return _joined; }

// ----- setter Functions -----
void Channel::setcName(const std::string& name) { _cName = name; }
void Channel::setcTopic(const std::string& topic) { _cTopic = topic; }
void Channel::setcPass(const std::string& password) { _cPass = password; }
void Channel::setChannelType() { _isInviteOnly = !_isInviteOnly; }

void Channel::addClient(Client *client) 
{ 
	for(size_t i = 0; i < _joined.size(); i++)
	{
		if (_joined[i] == client)
			return;
	}
	_joined.push_back(client);
}

void Channel::rmClient(int fd) {
    for (std::vector<Client*>::iterator it = _joined.begin(); it != _joined.end(); )
    {
        if ((*it)->getFd() == fd)
        {
            it = _joined.erase(it);
        } 
        else
        {
            ++it;
        }
    }
}

void Channel::rmOperator(int fd) {
    for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); )
    {
        if ((*it)->getFd() == fd)
        {
            it = _operators.erase(it);
        } 
        else
        {
            ++it;
        }
    }
}