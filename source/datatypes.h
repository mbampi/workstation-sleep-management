#ifndef DATATYPES_H
#define DATATYPES_H

#include <string.h>
#include <stdio.h>
#include <netinet/in.h>

using namespace std;

enum packet_type
{
    DISCOVERY_REQ = 0,
    DISCOVERY_RES = 1,

    STATUS_REQ = 2,
    STATUS_RES = 3,

    MONITORING_REQ = 4,
    MONITORING_RES = 5,

    EXIT_REQ = 6,
};

typedef struct
{
    uint16_t type;          // Tipo do pacote (p.ex. DISCOVERY | MONTIORING | EXIT)
    uint16_t seqn;          // Número de sequência do pacote
    string sender_ip;       // IP do sender
    string sender_hostname; // Hostname do sender
    string sender_mac;      // MAC do sender
} packet;

packet *decode_packet(string buffer);
string encode_packet(packet *p);

enum status
{
    awake = 0,
    asleep = 1
};

string status_to_string(status s);

typedef struct participant_info
{
    string hostname;
    string ip;
    string mac;
    status state;
    int rounds_without_activity;
} participant_info;

#endif // DATATYPES_H