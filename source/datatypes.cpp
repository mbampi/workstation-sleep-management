
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <netdb.h> // to use hostent
#include <netinet/in.h>
#include <arpa/inet.h>

#include "datatypes.h"

using namespace std;

string status_to_string(status s)
{
    switch (s)
    {
    case awake:
        return "awake";
    case asleep:
        return "asleep";
    default:
        return "unknown";
    }
}

// encode packet to string
string encode_packet(packet *p)
{
    string packet_str = "";
    packet_str += to_string(p->type);
    packet_str += "|" + to_string(p->seqn);
    packet_str += "|" + p->sender_ip;
    packet_str += "|" + p->sender_hostname;
    packet_str += "|" + p->sender_mac;
    return packet_str;
}

// decode packet from string
packet *decode_packet(string buffer)
{
    packet *p = new packet();
    char *token = strtok((char *)buffer.c_str(), "|");
    p->type = (packet_type)atoi(token);
    token = strtok(NULL, "|");
    p->seqn = atoi(token);
    token = strtok(NULL, "|");
    p->sender_ip = token;
    token = strtok(NULL, "|");
    p->sender_hostname = token;
    token = strtok(NULL, "|");
    p->sender_mac = token;

    return p;
}