#pragma once

#include "Client.hpp"
#include <string>
#include <vector>
#include <ctime>
#include <map>

class Bot : public Client 
{
    public:
        // Constructor
        Bot(int fd, const std::string& ip, const std::string& nick);
        ~Bot();

        // Methods
        void connectToServer(std::string pass);
        void joinChannel(const std::string& channel);
        void handleMessage(const std::string& message);
        void sendRandomPhrase();
        void sendRawMessage(const std::string& message);
        bool isConnected() const;
        bool isPending() const;

        // Setters
        void setConnected(bool connected);
        void setFd(int fd);
        void setPending(bool pending);

    private:
        // Constructors
        Bot();
        Bot(const Bot& obj);
        Bot& operator=(const Bot& rhs);

        // Attributes
        std::vector<std::string> phrases_;
        time_t joinTime_;
        bool isConnected_;
        bool isPending_;
        std::map<std::string, time_t> channelJoinTimes_;
        std::map<std::string, bool> channelInitialMessageSent_;
};