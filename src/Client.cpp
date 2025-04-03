#include "../inc/Client.hpp"

Client::Client() {}

Client::Client(int fd, const std::string& ip) : _fd(fd), _ip(ip) 
{
	ClientInit();
}

Client::Client(const Client& obj) 
{
	_fd = obj._fd;
	_ip = obj._ip;
	_nickname = obj._nickname;
	_username = obj._username;
	_realname = obj._realname;
	_hostname = obj._hostname;
	_status = obj._status;
	_isAdmin = obj._isAdmin;
	_passAccepted = obj._passAccepted;
	_pingSent = obj._pingSent;
	_lastActivity = obj._lastActivity;
	_invisible = obj._invisible;
	_buffer.clear();
	_channelList.clear();
}

Client::~Client() {}

Client& Client::operator=(const Client& rhs) {
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_ip = rhs._ip;
		_nickname = rhs._nickname;
		_username = rhs._username;
		_realname = rhs._realname;
		_hostname = rhs._hostname;
		_status = rhs._status;
		_isAdmin = rhs._isAdmin;
		_passAccepted = rhs._passAccepted;
		_pingSent = rhs._pingSent;
		_lastActivity = rhs._lastActivity;
		_invisible = rhs._invisible;
		_buffer = rhs._buffer;
		_channelList = rhs._channelList;
		_userModes = rhs._userModes;
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
	_hasQuit = false;
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
time_t Client::getPingSentTime() const { return _pingSentTime; }
bool Client::getRights() const { return _isAdmin; }
bool Client::getPassAccepted() const { return _passAccepted; }
bool Client::getPingSent() const { return _pingSent; }
bool Client::isInvisible() const { return _invisible; }
bool Client::getHasQuit() const { return _hasQuit; }
int  Client::getFd() const { return _fd; }
const std::vector<std::string>& Client::getChannelList() const { return _channelList; }

// ----- setter Functions -----
void Client::setUser(const std::string& name) { _username = name; }
void Client::setRealName(const std::string& name) { _realname = name; }
void Client::setNick(const std::string& name) { _nickname = name; }
void Client::setFd(int fd) { _fd = fd; }
void Client::setHasQuit(bool hasQuit) { _hasQuit = hasQuit; }
void Client::setStatus(AuthState status) { _status = status; }
void Client::setRights(bool isAdmin) { _isAdmin = isAdmin; }
void Client::setPassAccepted(bool passAccepted) { _passAccepted = passAccepted; }
void Client::setInvisible(bool invisible) { _invisible = invisible; }
void Client::setPingSent(bool pingSent) { _pingSent = pingSent; }
void Client::setLastActivity(time_t lastActivity) { _lastActivity = lastActivity; }
void Client::setPingSentTime(time_t pingSentTime) { _pingSentTime = pingSentTime; }

void Client::addChannelToList(const std::string& channelName) 
{ 
	_channelList.push_back(channelName); 
}

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
