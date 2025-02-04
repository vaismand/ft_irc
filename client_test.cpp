#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main() {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[1024] = "Hello, server!";

    // Create the client socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        return 1;
    }

    // Set up the server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6667); // Server port

    // Convert IP address and store
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(client_fd);
        return 1;
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client_fd);
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    // Send a message to the server
    if (send(client_fd, buffer, strlen(buffer), 0) < 0) {
        perror("send");
        close(client_fd);
        return 1;
    }

    std::cout << "Message sent: " << buffer << std::endl;

    // Close the socket
    close(client_fd);
    return 0;
}