
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

// Participant

enum status
{
    awake = 0,
    asleep = 1
};

string StatusToString(status s)
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

typedef struct __participant
{
    string ip;
    string mac;
    status status;
} participant;
