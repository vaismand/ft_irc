#pragma once

#include "Client.hpp"
#include <string>
#include <vector>
#include <ctime>

class Bot : public Client {
public:
    Bot(int fd, const std::string& ip, const std::string& nick);
    ~Bot();

    void connectToServer();
    void joinChannel(const std::string& channel);
    void handleMessage(const std::string& message);
    void sendRandomPhrase();

private:
    void sendRawMessage(const std::string& message);
    std::vector<std::string> phrases_;
    std::vector<std::string> channels_;
    time_t lastPhraseTime_;
    time_t joinTime_;
    bool initialDelayPassed_;
};