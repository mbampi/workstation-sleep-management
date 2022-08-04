

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

using namespace std;

typedef struct
{
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t clilen;
    char buf[256];
} udp_comm;

int initServer(udp_comm &server);
int send(udp_comm &server, const char *message);
int receive(udp_comm &server);

int initServer(udp_comm &server, int port)
{
    if ((server.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("ERROR opening socket");
        return -1;
    }

    server.serv_addr.sin_family = AF_INET;
    server.serv_addr.sin_port = htons(port);
    server.serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server.serv_addr.sin_zero), 8);

    if (bind(server.sockfd, (struct sockaddr *)&server.serv_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("ERROR on binding");
        return -1;
    }

    return 0;
}

int send(udp_comm &server, const char *message, int port)
{
    const char *ip = "localhost"; // TODO: broadcast
    struct hostent *host_server = gethostbyname(ip);
    if (host_server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)host_server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    char buffer[256];
    bzero(buffer, 256);
    strcpy(buffer, message);
    cout << "Sending message" << endl;

    int n = sendto(server.sockfd, message, strlen(message), 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    if (n < 0)
        printf("ERROR on send");
    else
        cout << "Message sent with " << n << " chars." << endl;
    return n;
}

// broadcast UDP message to all local network on port 4002
int broadcastMessage(string message)
{
    sockaddr_in si_me, si_other;
    int port = 4001;
    int broadcast = 1;

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == -1)
    {
        printf("ERROR opening socket");
        return -1;
    }
    cout << "usage hostname port " << port << endl;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast);

    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = INADDR_ANY;

    cout << "binding socket" << endl;
    bind(s, (sockaddr *)&si_me, sizeof(sockaddr));

    while (true)
    {
        printf("Send message to broadcast\n");
        char buf[1000];
        strcpy(buf, "testing123");
        unsigned slen = sizeof(sockaddr);
        send(s, buf, sizeof(buf) - 1, 0);
        // recvfrom(s, buf, sizeof(buf) - 1, 0, (sockaddr *)&si_other, &slen);
        // printf("recv: %s\n", buf);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int receive(udp_comm &server)
{
    struct sockaddr_in cli_addr;
    int n = recvfrom(server.sockfd, server.buf, 256, 0, (struct sockaddr *)&cli_addr, &server.clilen);
    if (n < 0)
        printf("ERROR on recvfrom");
    cout << "Message received: '" << server.buf << "' from " << cli_addr.sin_addr.s_addr << endl;
    return n;
}