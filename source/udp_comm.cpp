
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
#include <cassert>

#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include "udp_comm.h"
#include "mgmt_ss.h"
#include "participant.h"

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

packet_res *receivePacket(int on_port)
{
    int sockfd, n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buf[256];
    int true_int = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        printf("ERROR opening socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(on_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &true_int, sizeof(true_int)) < 0)
        printf("ERROR on setsockopt reuse port");

    if (::bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
        printf("ERROR on binding");

    clilen = sizeof(struct sockaddr_in);

    n = recvfrom(sockfd, buf, 256, 0, (struct sockaddr *)&cli_addr, &clilen);

    cout << "receivePacket: " << buf << endl;

    packet_res *p = decode_packet(buf, &cli_addr);

    if (n < 0)
    {
        printf("ERROR on recvfrom");
        p->sender_hostname = "NULL";
        p->sender_ip = inet_ntoa((&cli_addr)->sin_addr);
    }

    cout << "receivePacket: decoded packet type=" << p->type << " seqn=" << p->seqn << " length=" << p->length << " timestamp=" << p->timestamp << " payload=" << p->payload << endl;
    return p;
}

// broadcast UDP message to all local network
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
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int)) < 0)
        printf("ERROR on setsockopt");
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &broadcast, sizeof(int)) < 0)
        printf("ERROR on setsockopt");

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

    while (!stop_program)
    {
        std::cout << std::endl;
        char buf[10000];
        unsigned slen = sizeof(sockaddr);
        cout << "receiveBroadcast: listening for broadcast on port " << on_port << endl;

        int nrecv = recvfrom(s, buf, sizeof(buf), 0, (sockaddr *)&si_other, &slen);
        cout << "receiveBroadcast: rcvd packet " << buf << " with len=" << nrecv << endl;

        packet_res *rcvd_packet = decode_packet(buf, &si_other);
        cout << "receiveBroadcast: packet type=" << rcvd_packet->type
             << " | seqn=" << rcvd_packet->seqn
             << " | length=" << rcvd_packet->length
             << " | payload:" << rcvd_packet->payload << endl;

        if (rcvd_packet->type == DISCOVERY_REQ)
        {
            cout << "receiveBroadcast: received DISCOVERY_REQ packet." << endl;

            managerIP = inet_ntoa(si_other.sin_addr);

            packet *p = new packet();
            p->type = DISCOVERY_RES;
            p->seqn = rcvd_packet->seqn;
            p->payload = encode_participantpayload();

            char *ip = inet_ntoa(si_other.sin_addr);
            int sent_bytes = sendPacket(ip, MANAGER_PORT, p);
            cout << "receiveBroadcast: sent DISCOVERY_RES with " << sent_bytes << " bytes"
                 << " to ip:port=" << ip << ":" << MANAGER_PORT << endl;
        }
        if (rcvd_packet->type == MONITORING_REQ)
        {
            cout << "monitoringBroadcast: received MONITORING_REQ packet." << endl;

            packet *p = new packet();
            p->type = MONITORING_RES;
            p->seqn = rcvd_packet->seqn;
            p->payload = "notebook_1";

            char *ip = inet_ntoa(si_other.sin_addr);
            int sent_bytes = sendPacket(ip, MANAGER_PORT, p);
            cout << "receivePacket: sent MONITORING_RES with " << sent_bytes << " bytes"
                 << " to ip:port=" << ip << ":" << MANAGER_PORT << endl;
        }
    }
    return 0;
}

string exec(const char *cmd)
{
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        throw runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();
    return result;
}

string getMacAddr()
{
    string MAC = exec("ifconfig en1 | awk '/ether/{print $2}'");
    return MAC;
}

string getHostname()
{
    string hostname = exec("hostname");
    string::size_type pos = hostname.find('.');
    if (pos > 0)
        hostname = hostname.substr(0, pos);
    return hostname;
}
