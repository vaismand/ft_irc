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
    phrases_.push_back("I have an important message for you: 01001000 01101001 00100001");
    phrases_.push_back("How's evaluation going, looks fine, right?");
    phrases_.push_back("Nice to meet you, you can write me whatever you want, but don't expect the answer though...");
    phrases_.push_back("Have a great day....after you give 110 points to the project!");
    phrases_.push_back("I am a bot, I don't have feelings, but I can pretend to be your friend!");
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

void Bot::connectToServer(std::string pass) {
    sendRawMessage("PASS " + pass);
    sendRawMessage("NICK " + getNick());
    sendRawMessage("USER " + getNick() + " 0 * :" + getNick());
    for (size_t i = 0; i < _channelList.size(); ++i) {
        joinChannel(_channelList[i]);
    }
}

void Bot::joinChannel(const std::string& channel) {
    if (std::find(_channelList.begin(), _channelList.end(), channel) != _channelList.end())
        return;
    if (!isConnected_) {
        std::cerr << "Bot is not connected to the server." << std::endl;
        return;
    }
    if (getFd() == -1) {
        std::cerr << "Invalid file descriptor." << std::endl;
        return;
    }
    if (channel.empty()) {
        std::cerr << "Channel name cannot be empty." << std::endl;
        return;
    }
    sendRawMessage("JOIN " + channel);
    channelJoinTimes_[channel] = std::time(0);
    channelInitialMessageSent_[channel] = false;
}

void Bot::sendRawMessage(const std::string& message) {
    std::string msg = message + "\r\n";
    send(4, msg.c_str(), msg.length(), 0);
}

void Bot::sendRandomPhrase() {
    if (_channelList.empty())
        return;

    time_t now = std::time(0);
    
    for (size_t i = 0; i < _channelList.size(); ++i) {
        const std::string& channel = _channelList[i];
        
        if (!channelInitialMessageSent_[channel]) {
            if (std::difftime(now, channelJoinTimes_[channel]) >= 10) {
                std::string phrase = "Welcome to channel! I'm a dummy bot here to help!";
                sendRawMessage("NOTICE " + channel + " :" + phrase);
                channelInitialMessageSent_[channel] = true;
                channelJoinTimes_[channel] = now;
            }
            continue;
        }
        
        if (std::difftime(now, channelJoinTimes_[channel]) >= 40) {
            std::string phrase = phrases_[std::rand() % phrases_.size()];
            sendRawMessage("NOTICE " + channel + " :" + phrase);
            channelJoinTimes_[channel] = now;
        }
    }
}
