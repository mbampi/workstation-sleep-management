#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

#define PORT 4000

int initServer();

int startParticipant()
{
    // start server
    // listen to messages
    // TODO: listen to i/o commands

    int sockfd = initServer();
    if (sockfd < 0)
    {
        printf("ERROR opening socket");
        return -1;
    }

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);
    char buf[256];

    while (true)
    {
        /* receive from socket */
        int n = recvfrom(sockfd, buf, 256, 0, (struct sockaddr *)&cli_addr, &clilen);
        if (n < 0)
            printf("ERROR on recvfrom");
        printf("Received a datagram: %s\n", buf);

        /* send to socket */
        n = sendto(sockfd, "Got your message\n", 17, 0, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr));
        if (n < 0)
            printf("ERROR on sendto");
    }

    close(sockfd);
    return 0;
}

int initServer()
{
    struct sockaddr_in serv_addr;
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("ERROR opening socket");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("ERROR on binding");
        return -1;
    }

    return sockfd;
}