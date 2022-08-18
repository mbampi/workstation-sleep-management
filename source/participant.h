#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>

using namespace std;

enum status
{
    awake = 0,
    asleep = 1
};

string status_to_string(status s);

typedef struct participant
{
    string hostname;
    string ip;
    string mac;
    status state;
} participant;

#endif // PARTICIPANT_H