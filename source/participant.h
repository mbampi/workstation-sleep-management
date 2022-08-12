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

typedef struct __participant
{
    string ip;
    string mac;
    status status;
} participant;

int startParticipant();
#endif