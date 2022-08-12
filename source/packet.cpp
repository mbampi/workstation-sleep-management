
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include "packet.h"

using namespace std;

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