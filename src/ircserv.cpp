#include <iostream>
#include "../inc/Server.hpp"

bool g_running = true;

static void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        std::cerr << "Server shutting down..." << std::endl;
        g_running = false;
    } else if (signal == SIGQUIT) {
        std::cerr << "Server shutting down..." << std::endl;
        g_running = false;
    }
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port>" << " <password>" << std::endl;
		return 1;
	}
	std::signal(SIGINT, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	Server server(argv[1], argv[2]);
	try {
		server.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
