#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <string.h>

#include "udp_comm.h"

using namespace std;

extern char *managerIP;

enum status
{
    awake = 0,
    asleep = 1
};

string StatusToString(status s);

typedef struct participant
{
    string hostname;
    string ip;
    string mac;
    status state;
    int lost_packets = 0;
} participant;

typedef struct participant_res
{
    string hostname;
    string mac;
} participant_res;

participant *decode_participantpayload(string payload);
string encode_participantpayload();

int startParticipant();
void participantExit();
void participantInterface();
int broadcastSubservice();

#endif