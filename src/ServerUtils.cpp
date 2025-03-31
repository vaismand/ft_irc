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
		Client& lastMember = getClient(joined[0]);
        return lastMember.getNick() == getBot().getNick();
	}
	return false;
}

void Server::handleEmptyChannel(int fd, Channel* channel) {
	if (channel->getJoined().size() == 1)
		isLastMemberBot(fd, channel);
	rmChannel(channel->getcName());
}

bool Server::isLastMemberBot(int clientFd, Channel* channel)
{
    try {
        Client& lastMember = getClient(clientFd);
        if (lastMember.getNick() == getBot().getNick()) {
            getBot().sendRawMessage("PART " + channel->getcName());
            getBot().rmChannelInList(channel->getcName());
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting last client: " << e.what() << std::endl;
    }
    return false;
}
