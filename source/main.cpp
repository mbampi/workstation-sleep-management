#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <csignal>

#include "packet.h"
#include "udp_comm.h"
#include "mgmt_ss.h"
#include "participant.h"
#include "manager.h"

using namespace std;

void signalHandler(int signum)
{
    cout << "Signal " << signum << " received." << endl;
    stop_program = true;
}

int main(int argc, char *argv[])
{
    cout << ("Workstation Sleep Manager\n") << endl;

    debug_mode = false;

    bool isManager = false;
    if (argc > 1)
        isManager = (strcmp(argv[1], "manager") == 0);

    if (debug_mode)
        cout << ("Started") << endl;

    signal(SIGINT, signalHandler);  // Ctrl + C
    signal(SIGTERM, signalHandler); // kill
    signal(SIGABRT, signalHandler); // abort

    if (isManager)
    {
        if (debug_mode)
            cout << ("Manager mode") << endl;
        startManager();
        return 0;
    }
    else
    {
        if (debug_mode)
            cout << ("Participant mode") << endl;
        startParticipant();
        return 0;
    }
}
