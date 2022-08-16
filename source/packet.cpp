
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
    string packet_str = "";
    packet_str += to_string(p->type);
    packet_str += "|" + to_string(p->seqn);
    packet_str += "|" + to_string(p->payload.length());
    packet_str += "|" + to_string(p->timestamp);
    packet_str += "|" + p->sender_ip;
    packet_str += "|" + p->sender_hostname;
    packet_str += "|" + p->sender_mac;
    packet_str += "|" + p->payload;
    return packet_str;
}

// decode packet from string
packet *decode_packet(string buffer, sockaddr_in *sender)
{
    packet *p = new packet();
    char *token = strtok((char *)buffer.c_str(), "|");
    p->type = (packet_type)atoi(token);
    token = strtok(NULL, "|");
    p->seqn = atoi(token);
    token = strtok(NULL, "|");
    p->length = atoi(token);
    token = strtok(NULL, "|");
    p->timestamp = atoi(token);
    token = strtok(NULL, "|");
    p->sender_ip = token;
    token = strtok(NULL, "|");
    p->sender_hostname = token;
    token = strtok(NULL, "|");
    p->sender_mac = token;
    token = strtok(NULL, "|");
    token[p->length] = '\0';
    p->payload = token;

    return p;
}
