
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>

#include "participant.h"

using namespace std;

participant *decode_participant_payload(char *payload)
{
    participant *p = new participant();
    char *token = strtok(payload, ";");
    p->hostname = token;
    token = strtok(NULL, ";");
    p->ip = token;
    token = strtok(NULL, ";");
    p->mac = token;
    token = strtok(NULL, ";");
    p->status = (status)atoi(token);
    return p;
}

string encode_participant_payload(participant *p)
{
    string payload = p->hostname + ";" + p->ip + ";" + p->mac + ";" + to_string(p->status);
    return payload;
}

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

int startParticipant()
{
    int n = receiveBroadcast(PARTICIPANT_PORT);
    cout << "Received broadcast " << n << endl;

    cout << "ending participant" << endl;

    return 0;
}
