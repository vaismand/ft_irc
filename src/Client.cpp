#include "../inc/Client.hpp"

Client::Client(int fd, const std::string& ip) : _fd(fd), _ip(ip) 
{
	ClientInit();
}

Client::Client(const Client& ref) : _fd(ref._fd), _ip(ref._ip) {}

Client::~Client() {}

Client& Client::operator=(const Client& rhs) {
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_ip = rhs._ip;
		_nickname = rhs._nickname;
		_username = rhs._username;
		_realname = rhs._realname;
	}
	return *this;
}

void Client::ClientInit() {
	_username = "";
	_realname = "";
	_hostname = "localhost";
	_nickname = "*";
	_status = UNREGISTERED;
	_isAdmin = false;
	_passAccepted = false;
	_pingSent = false;
	_lastActivity = time(NULL);
	_invisible = false;
}

// ----- getter Functions -----
std::string Client::getUser() const { return _username; }
std::string Client::getRealName() const { return _realname; }
std::string Client::getHostName() const { return _hostname; }
std::string Client::getNick() const { return _nickname; }
std::string Client::getIp() const { return _ip; }
std::string& Client::getBuffer() { return _buffer; }
std::string Client::getUserModes() const { return _userModes; }
AuthState Client::getStatus() const { return _status; }
time_t Client::getLastActivity() const { return _lastActivity; }
bool Client::getRights() const { return _isAdmin; }
bool Client::getPassAccepted() const { return _passAccepted; }
bool Client::getPingSent() const { return _pingSent; }
bool Client::isInvisible() const { return _invisible; }
int  Client::getFd() const { return _fd; }
const std::vector<std::string>& Client::getChannelList() const { return _channelList; }

// ----- setter Functions -----
void Client::setUser(const std::string& name) { _username = name; }
void Client::setRealName(const std::string& name) { _realname = name; }
void Client::setNick(const std::string& name) { _nickname = name; }
void Client::setStatus(AuthState status) { _status = status; }
void Client::setRights() { _isAdmin = !_isAdmin; }
void Client::setPassAccepted(bool passAccepted) { _passAccepted = passAccepted; }
void Client::setInvisible(bool invisible) { _invisible = invisible; }
void Client::setPingSent(bool pingSent) { _pingSent = pingSent; }
void Client::setLastActivity(time_t lastActivity) { _lastActivity = lastActivity; }
void Client::setChannelList(const std::string& channelName) { _channelList.push_back(channelName); }

void Client::setUserModes() {
	_userModes.clear();
	_userModes = "+";
	if (_invisible)
		_userModes += "i";
 }

// ----- methods -----
void Client::rmChannelInList(const std::string& channelName) {
	std::vector<std::string>::iterator it = std::find(_channelList.begin(), _channelList.end(), channelName);
	if (it != _channelList.end()) {
		_channelList.erase(it);
	}
}
