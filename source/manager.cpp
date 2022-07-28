
#include <stdio.h>
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
#include <iostream> // std::cout
#include <map>      // to use map
#include "datatypes.h"
#include <iomanip> // to use setw

using namespace std;

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
void populateFakeParticipants();
void printParticipants();

map<string, participant> participants_map; // hostname -> participant

int startManager()
{
    // discovery subservice
    thread discoveryThread(discoverySubservice);

    // TODO: monitoring subservice
    // TODO: management subservice
    // TODO: interface subservice

    cout << "Enter to stop" << endl;
    cin.get();

    discoveryThread.join();

    populateFakeParticipants();
    printParticipants();

    cout << ("Manager stopped") << endl;
    return 0;
}

int discoverySubservice()
{
    cout << "Started DiscoverySubservice" << endl;
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
    cout << "Sending message" << endl;

    int n = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
    if (n < 0)
        printf("ERROR sendto");

    unsigned int length = sizeof(struct sockaddr_in);
    struct sockaddr_in from;
    n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *)&from, &length);
    if (n < 0)
        cout << "ERROR recvfrom";

    cout << "Got an ack: " << buffer << endl;

    close(sockfd);
    return 0;
}

//
// UTILITIES

void printParticipants()
{
    cout << setw(15) << "PARTICIPANTS" << endl;
    cout << left << setw(12)
         << "Hostname"
         << left << setw(12)
         << "IP"
         << left << setw(20)
         << "MAC"
         << left << setw(10)
         << "Status"
         << endl;
    for (const auto &[h, p] : participants_map)
    {
        cout
            << left << setw(12)
            << h
            << left << setw(12)
            << p.ip
            << left << setw(20)
            << p.mac
            << left << setw(10)
            << StatusToString(p.status)
            << endl;
    }
}

void populateFakeParticipants()
{
    participant p1;
    p1.ip = "1.1.1.1";
    p1.mac = "01:01:01:01:01:01";
    p1.status = awake;
    participants_map["host1"] = p1;

    participant p2;
    p2.ip = "1.1.1.2";
    p2.mac = "02:02:02:02:02:02";
    p2.status = asleep;
    participants_map["host2"] = p2;

    participant p3;
    p3.ip = "1.1.1.3";
    p3.mac = "03:03:03:03:03:03";
    p3.status = asleep;
    participants_map["host3"] = p3;
}