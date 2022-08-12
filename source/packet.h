

#ifndef PACKET_H
#define PACKET_H
#include <string.h>
#include <stdio.h>

using namespace std;

enum packet_type
{
    DISCOVERY_REQ = 0,
    DISCOVERY_RES = 1,

    STATUS_REQ = 2,
    STATUS_RES = 3,

};

typedef struct __packet
{
    uint16_t type;        // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;        // Número de sequência
    uint16_t length;      // Comprimento do payload
    uint16_t timestamp;   // Timestamp do dado
    const char *_payload; // Dados da mensagem
} packet;

packet *decode_packet(string buffer);
string encode_packet(packet *p);

#endif
