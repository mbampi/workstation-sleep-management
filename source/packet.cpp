
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;

// Packet

enum packet_type
{
    sleep_service_discovery = 0,
    sleep_status_request = 1
};

typedef struct __packet
{
    uint16_t type;        // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;        // Número de sequência
    uint16_t length;      // Comprimento do payload
    uint16_t timestamp;   // Timestamp do dado
    const char *_payload; // Dados da mensagem
} packet;

// encode packet to char array
char *encode_packet(packet *p)
{
    char *buffer = (char *)malloc(sizeof(packet));
    memcpy(buffer, p, sizeof(packet));
    return buffer;
}

// decode char array to packet
packet *decode_packet(char *buffer)
{
    packet *p = (packet *)malloc(sizeof(packet));
    memcpy(p, buffer, sizeof(packet));
    return p;
}

// get packet type from char array
packet_type get_packet_type(char *buffer)
{
    return (packet_type)buffer[0];
}