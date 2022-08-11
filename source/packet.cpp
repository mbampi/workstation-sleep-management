
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;

// Packet

enum packet_type
{
    DISCOVERY = 0,
    REQUEST = 1
};

typedef struct __packet
{
    uint16_t type;        // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;        // Número de sequência
    uint16_t length;      // Comprimento do payload
    uint16_t timestamp;   // Timestamp do dado
    const char *_payload; // Dados da mensagem
} packet;

int encode_packet(packet *p, char *buffer);
packet *decode_packet(char *buffer);

// encode packet to char array
int encode_packet(packet *p, char *buffer)
{
    p->length = strlen(p->_payload);
    int len = sizeof(p->type) + sizeof(p->seqn) + sizeof(p->length) + sizeof(p->timestamp) + p->length;
    buffer = (char *)malloc(len);
    cout << "buffer len: " << len << endl;
    memcpy(buffer, p, len);
    return len;
}

// decode char array to packet
packet *decode_packet(char *buffer, int len)
{
    packet *p = (packet *)malloc(sizeof(packet));
    memcpy(p, buffer, len);
    return p;
}

// encode packet to char array
string encode_packet_str(packet *p)
{
    return to_string(p->type) + "|" + to_string(p->seqn) + "|" + to_string(p->length) + "|" + to_string(p->timestamp) + "|" + p->_payload;
}

// split string by |
packet *decode_packet_str(string buffer)
{
    // split string by |
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