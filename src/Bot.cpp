#include "../inc/Bot.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

Bot::Bot(int fd, const std::string& ip, const std::string& nick)
    : Client(fd, ip), lastPhraseTime_(0), joinTime_(std::time(0)), initialDelayPassed_(false) {
    setNick(nick);
    phrases_.push_back("Hello, world!");
    phrases_.push_back("How's it going?");
    phrases_.push_back("Nice to meet you!");
    phrases_.push_back("Have a great day!");
    std::srand(std::time(0));
}

Bot::~Bot() {}

void Bot::connectToServer() {
    sendRawMessage("NICK " + getNick());
    sendRawMessage("USER " + getNick() + " 0 * :" + getNick());
    for (size_t i = 0; i < channels_.size(); ++i) {
        joinChannel(channels_[i]);
    }
}

void Bot::joinChannel(const std::string& channel) {
    sendRawMessage("JOIN " + channel);
    channels_.push_back(channel);
    joinTime_ = std::time(0);
}

void Bot::handleMessage(const std::string& message) {
    if (message.find("PING") == 0) {
        std::cout << "Bot responding to PING" << std::endl;
        sendRawMessage("PONG " + message.substr(5));
    }
}

void Bot::sendRawMessage(const std::string& message) {
    std::string msg = message + "\r\n";
    send(getFd(), msg.c_str(), msg.length(), 0);
}

void Bot::sendRandomPhrase() {
    if (channels_.empty())
        return;

    time_t now = std::time(0);

    if (!initialDelayPassed_) {
        if (std::difftime(now, joinTime_) >= 8) {
            for (size_t i = 0; i < channels_.size(); ++i) {
                sendRawMessage("PRIVMSG " + channels_[i] + " : Welcome to the channel!");
            }
            initialDelayPassed_ = true;
            lastPhraseTime_ = now;
        } else {
            return;
        }
    }

    if (std::difftime(now, lastPhraseTime_) >= 60) {
        lastPhraseTime_ = now;
        std::string phrase = phrases_[std::rand() % phrases_.size()];
        for (size_t i = 0; i < channels_.size(); ++i) {
            sendRawMessage("PRIVMSG " + channels_[i] + " :" + phrase);
        }
    }
}
