#include <iostream>
#include "../inc/Server.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port>" << " <password>" << std::endl;
		return 1;
	}
	Server server(argv[1], argv[2]);
	try {
		server.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
