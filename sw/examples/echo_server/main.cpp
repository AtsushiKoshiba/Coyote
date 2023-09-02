#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    // Check the number of arguments.
    if (argc != 2)
    {
        std::cerr << "Usage: echo_server <port>" << std::endl;
        return 1;
    }

    // Get the port number from the command line argument.
    int port = std::atoi(argv[1]);

    // Create a socket.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Bind the socket to a port.
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    // Listen for connections.
    listen(sockfd, 5);

    char buffer[64];

    // Accept connections.
    while (true)
    {
        int accept_fd = accept(sockfd, nullptr, nullptr);
        if (accept_fd < 0)
        {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        // Read the message from the client.
        int n = recv(accept_fd, buffer, sizeof(buffer), 0);
        if (n < 0)
        {
            std::cerr << "Error receiving message" << std::endl;
            close(accept_fd);
            continue;
        }

        // Echo the message back to the client.
        send(accept_fd, buffer, n, 0);

        // Close the connection.
        close(accept_fd);
    }

    return 0;
}
