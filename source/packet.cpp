
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;

// Packet

enum packet_type
{
    DISCOVERY = 0,
    DISCOVERY_ACK = 1,
};

typedef struct __packet
{
    uint16_t type;        // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;        // Número de sequência
    uint16_t length;      // Comprimento do payload
    uint16_t timestamp;   // Timestamp do dado
    const char *_payload; // Dados da mensagem
} packet;

// encode packet to string
string encode_packet(packet *p)
{
    return to_string(p->type) + "|" + to_string(p->seqn) + "|" + to_string(p->length) + "|" + to_string(p->timestamp) + "|" + p->_payload;
}

// decode packet from string
packet *decode_packet(string buffer)
{
    packet *p = (packet *)malloc(sizeof(packet));
    p->type = atoi(buffer.substr(0, buffer.find("|")).c_str());
    buffer = buffer.substr(buffer.find("|") + 1);
    p->seqn = atoi(buffer.substr(0, buffer.find("|")).c_str());
    buffer = buffer.substr(buffer.find("|") + 1);
    p->length = atoi(buffer.substr(0, buffer.find("|")).c_str());
    buffer = buffer.substr(buffer.find("|") + 1);
    p->timestamp = atoi(buffer.substr(0, buffer.find("|")).c_str());
    buffer = buffer.substr(buffer.find("|") + 1);
    p->_payload = buffer.c_str();
    return p;
}