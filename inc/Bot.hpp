#pragma once

#include "Client.hpp"
#include <string>
#include <vector>
#include <ctime>
#include <map>

class Bot : public Client {
public:
    Bot(int fd, const std::string& ip, const std::string& nick);
    ~Bot();

    void connectToServer();
    void joinChannel(const std::string& channel);
    void handleMessage(const std::string& message);
    void sendRandomPhrase();
    void sendRawMessage(const std::string& message);
    void rmBotFromChannel(Server &server, Client &client, const std::string& channel);

private:
    std::vector<std::string> phrases_;
    time_t joinTime_;
    std::map<std::string, time_t> channelJoinTimes_;
    std::map<std::string, bool> channelInitialMessageSent_;
};