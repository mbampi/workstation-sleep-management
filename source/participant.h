#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>

#include "udp_comm.h"

using namespace std;

enum status
{
    awake = 0,
    asleep = 1
};

string StatusToString(status s);

typedef struct
{
    string hostname;
    string ip;
    string mac;
    status state;
    int lost_packets = 0;
} participant;

typedef struct
{
    string hostname;
    string mac;
} participant_res;

participant *decode_participant_payload(char *payload);
string encode_participant_payload(participant *p);

int startParticipant();

#endif