
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <netdb.h> // to use hostent
#include <netinet/in.h>
#include <arpa/inet.h>

#include "participant.h"
#include "packet.h"

using namespace std;

// encode packet to string
string encode_packet(packet *p)
{
    return to_string(p->type) + "|" + to_string(p->seqn) + "|" + to_string(p->payload.length()) + "|" + to_string(p->timestamp) + "|" + p->payload;
}

// decode packet from string
packet_res *decode_packet(string buffer, sockaddr_in *sender)
{
    packet_res *p = new packet_res();
    char *token = strtok((char *)buffer.c_str(), "|");
    p->type = (packet_type)atoi(token);
    token = strtok(NULL, "|");
    p->seqn = atoi(token);
    token = strtok(NULL, "|");
    p->length = atoi(token);
    token = strtok(NULL, "|");
    p->timestamp = atoi(token);
    token = strtok(NULL, "|");
    token[p->length] = '\0';
    p->payload = token;

    if (p->type == DISCOVERY_RES)
    {
        participant *part = decode_participantpayload(p->payload);
        p->sender_hostname = part->hostname;
        p->sender_mac = part->mac;
    }

    char *ip = inet_ntoa(sender->sin_addr);
    p->sender_ip = ip;

    return p;
}
