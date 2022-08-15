#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "packet.h"
#include "udp_comm.h"
#include "participant.h"
#include "manager.h"

using namespace std;

void signalHandler(int signum)
{
    cout << "Signal " << signum << " received." << endl;
}

int main(int argc, char *argv[])
{
    cout << ("Workstation Sleep Manager\n") << endl;

    bool isManager = false;
    if (argc > 1)
        isManager = (strcmp(argv[1], "manager") == 0);

    cout << ("Started") << endl;

    signal(SIGINT, signalHandler);  // Ctrl + C
    signal(SIGTERM, signalHandler); // kill
    signal(SIGABRT, signalHandler); // abort

    if (isManager)
    {
        cout << ("Manager mode") << endl;
        startManager();
        return 0;
    }
    else
    {
        cout << ("Participant mode") << endl;
        startParticipant();
        return 0;
    }
}
