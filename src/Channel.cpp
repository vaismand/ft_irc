
#include "../inc/Channel.hpp"

Channel::Channel() {}

Channel::~Channel() {}



// ----- getter Functions -----
std::string Channel::getcName() const { return _cName; }
std::string Channel::getcTopic() const { return _cTopic; }
std::string Channel::getcPass() const { return _cPass; }
bool Channel::getChannelType() const { return _isInviteOnly; }

// ----- setter Functions -----
void Channel::setcName(const std::string& name) { _cName = name; }
void Channel::setcTopic(const std::string& topic) { _cTopic = topic; }
void Channel::setcPass(const std::string& password) { _cPass = password; }
void Channel::setChannelType() { _isInviteOnly = !_isInviteOnly; }