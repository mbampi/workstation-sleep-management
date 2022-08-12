
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <netdb.h> // to use hostent
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "udp_comm.h"

using namespace std;

int sendPacket(char *ip, int port, packet *p)
{
    int sockfd, n;
    struct sockaddr_in serv_addr, cli_addr;

    struct hostent *server = gethostbyname(ip);
    if (server == NULL)
        fprintf(stderr, "ERROR, no such host\n");

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        printf("ERROR opening socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    string encoded_msg = encode_packet(p);
    int len = strlen(encoded_msg.c_str());
    cout << "sendPacket=" << encoded_msg << " with len=" << len << endl;
    n = sendto(sockfd, encoded_msg.c_str(), len, 0, (const struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
    if (n < 0)
        printf("ERROR sendto");
    return n;
}

packet *receivePacket(int on_port)
{
    int sockfd, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buf[256];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        printf("ERROR opening socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(on_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
        printf("ERROR on binding");

    clilen = sizeof(struct sockaddr_in);

    n = recvfrom(sockfd, buf, 256, 0, (struct sockaddr *)&cli_addr, &clilen);
    if (n < 0)
        printf("ERROR on recvfrom");

    packet *p = decode_packet(buf);
    return p;
}

// broadcast UDP message to all local network on port 4001
int broadcastMessage(string msg, int port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    int broadcast = 1;
    int errors = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, '\n', sizeof(addr.sin_zero));

    int len = strlen(msg.c_str());
    int bytes_sent = sendto(fd, msg.c_str(), len, 0, (struct sockaddr *)&addr, sizeof(addr));
    return bytes_sent;
}

int broadcastPacket(packet *msg, int port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    int broadcast = 1;
    int errors = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, '\n', sizeof(addr.sin_zero));

    string encoded_msg = encode_packet(msg);
    int len = strlen(encoded_msg.c_str());
    cout << "broadcastPacket=" << encoded_msg << " with len=" << len << endl;
    int bytes_sent = sendto(fd, encoded_msg.c_str(), len, 0, (struct sockaddr *)&addr, sizeof(addr));

    return bytes_sent;
}

int receiveBroadcast(int on_port)
{
    sockaddr_in si_me, si_other;
    int broadcast = 1;
    int s;
    assert((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1);

    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast);

    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(on_port);
    si_me.sin_addr.s_addr = INADDR_ANY;

    assert(::bind(s, (sockaddr *)&si_me, sizeof(sockaddr)) != -1);

    while (true)
    {
        std::cout << std::endl;
        char buf[10000];
        unsigned slen = sizeof(sockaddr);
        cout << "listening for broadcast on port " << on_port << endl;

        int nrecv = recvfrom(s, buf, sizeof(buf), 0, (sockaddr *)&si_other, &slen);
        cout << "DEBUG: rcvd packet " << buf << " with len=" << nrecv << endl;

        bool string_msg = false;
        if (!string_msg)
        {
            packet *p = decode_packet(buf);
            cout << "DEBUG: packet type=" << p->type
                 << " | seqn=" << p->seqn
                 << " | length=" << p->length
                 << " | payload:" << p->_payload << endl;

            if (p->type == DISCOVERY_REQ)
            {
                cout << "DEBUG: received DISCOVERY_REQ packet." << endl;
                p->type = DISCOVERY_RES;
                char *ip = inet_ntoa(si_other.sin_addr);
                int sent_bytes = sendPacket(ip, MANAGER_PORT, p);
                cout << "DEBUG: sent DISCOVERY_RES with " << sent_bytes << " bytes"
                     << " to ip:port=" << ip << ":" << MANAGER_PORT << endl;
            }
        }
    }
}
