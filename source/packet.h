

#ifndef PACKET_H
#define PACKET_H
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
    uint16_t type;          // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;          // Número de sequência
    uint16_t length;        // Comprimento do payload
    uint16_t timestamp;     // Timestamp do dado
    string sender_ip;       // IP do sender
    string sender_hostname; // Hostname do sender
    string sender_mac;      // MAC do sender
    string payload;         // Dados da mensagem
    bool flag_monitoring = false; // Flag para tentar resolver monitoring
} packet;

packet *decode_packet(string buffer, sockaddr_in *sender);
string encode_packet(packet *p);

#endif
