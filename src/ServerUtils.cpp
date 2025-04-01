#include "../inc/Server.hpp"


void Server::setPollfd(int fd, short int events, std::vector<struct pollfd> &pollfds)
{
	struct pollfd pollfd;
	pollfd.fd = fd;
	pollfd.events = events;
	pollfd.revents = 0;
	pollfds.push_back(pollfd);
}

void Server::rmPollfd(int fd) 
{
    for (std::vector<struct pollfd>::reverse_iterator it = _pollfds.rbegin(); it != _pollfds.rend(); ++it) 
    {
        if (it->fd == fd) 
        {
            _pollfds.erase(it.base() - 1);
            break;
        }
    }
}

bool Server::isChannelEmptyOrBotOnly(Channel* channel) const {
	if (!channel)
		return false;
	const std::vector<int>& joined = channel->getJoined();
	if (joined.empty())
		return true;
	if (joined.size() == 1) {
		Client* lastMember = _clients.at(joined[0]);
		return lastMember == bot_;
    }
	return false;
}

void Server::handleEmptyChannel(Channel* channel) {
	if (!channel)
		return;

	if (isChannelEmptyOrBotOnly(channel)) {
		const std::vector<int>& joined = channel->getJoined();
		if (joined.size() == 1 && _clients.at(joined[0]) == bot_) {
			bot_->sendRawMessage("PART " + channel->getcName());
			bot_->rmChannelInList(channel->getcName());
		}
		rmChannel(channel->getcName());
	}
}
