#include "../inc/Bot.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

//Constructors
Bot::Bot() {}

Bot::Bot(int fd, const std::string& ip, const std::string& nick)
    : Client(fd, ip), joinTime_(std::time(0)), isConnected_(false), isPending_(true) {
    setNick(nick);
    phrases_.push_back("Hello, world!");
    phrases_.push_back("How's it going?");
    phrases_.push_back("Nice to meet you!");
    phrases_.push_back("Have a great day!");
    std::srand(std::time(0));
    //_fd = -1;
}

Bot::Bot(const Bot& obj) : Client(obj), phrases_(obj.phrases_), joinTime_(obj.joinTime_) {
    *this = obj;
}

Bot& Bot::operator=(const Bot& rhs) {
    if (this != &rhs) {
        Client::operator=(rhs);
        phrases_ = rhs.phrases_;
        joinTime_ = rhs.joinTime_;
    }
    return *this;
}

Bot::~Bot() {}

bool Bot::isConnected() const {
    return isConnected_;
}

bool Bot::isPending() const {
    return isPending_;
}

void Bot::setPending(bool pending) {
    isPending_ = pending;
}

void Bot::setFd(int fd) {
    _fd = fd;
    isConnected_ = true;
}

void Bot::setConnected(bool connected) {
    isConnected_ = connected;
}

void Bot::connectToServer() {
    sendRawMessage("NICK " + getNick());
    sendRawMessage("USER " + getNick() + " 0 * :" + getNick());
    for (size_t i = 0; i < _channelList.size(); ++i) {
        joinChannel(_channelList[i]);
    }
}

void Bot::joinChannel(const std::string& channel) {
    sendRawMessage("JOIN " + channel);
    if (std::find(_channelList.begin(), _channelList.end(), channel) == _channelList.end()) {
        _channelList.push_back(channel);
    }
    channelJoinTimes_[channel] = std::time(0);
    channelInitialMessageSent_[channel] = false;
}

void Bot::handleMessage(const std::string& message) {
    if (message.find("PING") == 0) {
        sendRawMessage("PONG " + message.substr(5));
    }
}

void Bot::sendRawMessage(const std::string& message) {
    std::string msg = message + "\r\n";
    send(getFd(), msg.c_str(), msg.length(), 0);
}

void Bot::sendRandomPhrase() {
    if (_channelList.empty())
        return;

    time_t now = std::time(0);
    
    for (size_t i = 0; i < _channelList.size(); ++i) {
        const std::string& channel = _channelList[i];
        
        if (!channelInitialMessageSent_[channel]) {
            if (std::difftime(now, channelJoinTimes_[channel]) >= 10) {
                std::string phrase = phrases_[std::rand() % phrases_.size()];
                sendRawMessage("PRIVMSG " + channel + " :" + phrase);
                channelInitialMessageSent_[channel] = true;
                channelJoinTimes_[channel] = now;
            }
            continue;
        }
        
        if (std::difftime(now, channelJoinTimes_[channel]) >= 40) {
            std::string phrase = phrases_[std::rand() % phrases_.size()];
            sendRawMessage("PRIVMSG " + channel + " :" + phrase);
            channelJoinTimes_[channel] = now;
        }
    }
}
