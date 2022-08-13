

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

};

typedef struct
{
    uint16_t type;        // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;        // Número de sequência
    uint16_t length;      // Comprimento do payload
    uint16_t timestamp;   // Timestamp do dado
    const char *_payload; // Dados da mensagem
} packet;

typedef struct
{
    uint16_t type;      // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;      // Número de sequência
    uint16_t length;    // Comprimento do payload
    uint16_t timestamp; // Timestamp do dado
    string _payload;    // Dados da mensagem

    string sender_ip;       // IP do sender
    string sender_mac;      // MAC do sender
    string sender_hostname; // Hostname do sender
} packet_res;

packet_res *decode_packet(string buffer, sockaddr_in *sender);
string encode_packet(packet *p);

#endif
