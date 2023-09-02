#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>

int main(int argc, char **argv)
{
    // Get the server IP address and port from the command line arguments.
    if (argc != 3)
    {
        std::cout << "Usage: echo_client <ip_address> <port>" << std::endl;
        return 1;
    }

    std::string ip_address = argv[1];
    int port = std::atoi(argv[2]);

    // Create a socket.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cout << "Error creating socket" << std::endl;
        return 1;
    }

    // Connect to the echo server.
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_aton(ip_address.c_str(), &serveraddr.sin_addr);
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        std::cout << "Error connecting to server" << std::endl;
        return 1;
    }

    // Fill data
    char buffer[64];
    for (int i = 0; i < 64; i++)
    {
        buffer[i] = 'x';
    }

    std::cout << "Buffer size: " << sizeof(buffer) << std::endl;

    // Benchmark the echo server.
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; i++)
    {
        // MSG_NOSIGNAL flag tells the system not to send a SIGPIPE signal
        // to the process if the other end of the socket has been closed.
        send(sockfd, buffer, sizeof(buffer), MSG_NOSIGNAL);
        recv(sockfd, buffer, sizeof(buffer), 0);
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    // Print the results of the benchmark.
    std::cout << "1000 requests in " << elapsed_seconds.count() << " seconds" << std::endl;
    std::cout << "Throughput: " << 1 / elapsed_seconds.count() << " requests/second" << std::endl;

    // Close the socket.
    close(sockfd);

    return 0;
}
