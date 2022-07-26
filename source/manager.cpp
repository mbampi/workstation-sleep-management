#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <errno.h>
#include <stdlib.h>

#define PORT 4000

#define handle_error_en(en, msg) \
    do                           \
    {                            \
        errno = en;              \
        perror(msg);             \
        exit(EXIT_FAILURE);      \
    } while (0)

#define handle_error(msg)   \
    do                      \
    {                       \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

int discoverySubservice();

int startManager()
{
    // discovery subservice
    std::thread discoveryThread(discoverySubservice);

    // TODO: monitoring subservice
    // TODO: management subservice
    // TODO: interface subservice

    std::cout << "Enter to stop" << std::endl;
    std::cin.get();

    discoveryThread.join();

    std::cout << ("Manager stopped") << std::endl;
    return 0;
}

int discoverySubservice()
{
    std::cout << "Started DiscoverySubservice" << std::endl;
    const char *ip = "localhost"; // TODO: get all computers in the same network
    struct hostent *server = gethostbyname(ip);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        printf("ERROR opening socket");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    char buffer[256];
    bzero(buffer, 256);
    strcpy(buffer, "Hello, world!");
    std::cout << "Sending message" << std::endl;

    int n = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
    if (n < 0)
        printf("ERROR sendto");

    unsigned int length = sizeof(struct sockaddr_in);
    struct sockaddr_in from;
    n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *)&from, &length);
    if (n < 0)
        printf("ERROR recvfrom");

    printf("Got an ack: %s\n", buffer);

    close(sockfd);
    return 0;
}
