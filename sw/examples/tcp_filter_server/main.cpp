#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

const char data_pattern[] = {0x68, 0x74, 0x74, 0x70, 0x73};
const std::string pattern_string(data_pattern, data_pattern + 5);

size_t tcp_data_filter(uint8_t *buffer, size_t buf_size)
{
    std::string packet_data(buffer, buffer + buf_size);
    return packet_data.find(pattern_string);
}

int main(int argc, char **argv)
{
    // Check the number of arguments.
    if (argc != 2)
    {
        std::cerr << "Usage: tcp_filter_server <port>" << std::endl;
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

    uint8_t buffer[64];
    int accept_fd;

    while (true)
    {
        accept_fd = accept(sockfd, nullptr, nullptr);
        if (accept_fd < 0)
        {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }
        else
        {
            break;
        }
    }

    // Accept connections.
    while (true)
    {
        // Read the message from the client.
        int n = recv(accept_fd, buffer, sizeof(buffer), 0);

        // Filter based on date
        tcp_data_filter(buffer, sizeof(buffer));

        if (n < 0)
        {
            std::cerr << "Error receiving message" << std::endl;
            close(accept_fd);
            break;
        }
    }

    return 0;
}
