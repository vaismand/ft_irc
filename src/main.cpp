#include "../inc/Server.hpp"

volatile bool g_running = true;

static void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        g_running = false;
    } else if (signal == SIGQUIT) {
        g_running = false;
    }
}



bool argCheck(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port>" << " <password>" << std::endl;
		return false;
	}
	int port;
	if (!dvais::stoi(argv[1], port)) {
		std::cerr << "Invalid port: " << argv[1] << std::endl;
		return false;
	}
	if (port < 6660 || port > 6669) {
		std::cerr << "Please use valid ports [6660 - 6669]" << std::endl;
		return false;
	}
	std::string password = argv[2];
	if (password.size() > 16 || password.size() < 4 || password.empty() \
	|| password.find(" ") != std::string::npos) {
		std::cerr << "Invalid Password: Password should be 4-16 characters, no spaces." << std::endl;
		return false;
	}
	for (std::size_t i = 0; i < password.size(); i++) {
		if (!isascii(password[i])) {
			std::cerr << "Invalid Password. Only ASCII characters are allowed." << std::endl;
			return false;
		}
	}
	return true;
}


int main(int argc, char **argv)
{
	if (!argCheck(argc, argv))
		return 1;
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
