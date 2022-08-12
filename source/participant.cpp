
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>

#include "participant.h"

using namespace std;

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
