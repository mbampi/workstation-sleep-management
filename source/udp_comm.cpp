

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

using namespace std;

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

    string encoded_msg = encode_packet_str(msg);
    int len = strlen(encoded_msg.c_str());
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
        char buf[10000];
        unsigned slen = sizeof(sockaddr);
        cout << "listening for broadcast on port " << on_port << endl;

        int nrecv = recvfrom(s, buf, sizeof(buf), 0, (sockaddr *)&si_other, &slen);
        cout << "DEBUG: rcvd packet with len=" << nrecv << endl;

        bool string_msg = false;
        if (string_msg)
        {
            cout << "DEBUG msg received: " << buf << endl;
        }
        else
        {
            packet *p = decode_packet_str(string(buf));
            cout << "DEBUG: packet type: " << p->type << endl;
            cout << "DEBUG: packet seqn: " << p->seqn << endl;
            cout << "DEBUG: packet length: " << p->length << endl;
            cout << "DEBUG: packet payload:" << p->_payload << endl;
        }
    }
}