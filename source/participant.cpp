
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <thread>
#include <algorithm>

#include "packet.h"
#include "udp_comm.h"
#include "mgmt_ss.h"
#include "participant.h"

using namespace std;

char *managerIP;

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
    cout << "Started Participant" << endl;

    thread broadcastReceiverThread(broadcastSubservice);

    participantInterface();

    broadcastReceiverThread.join();

    cout << "ending participant" << endl;

    return 0;
}

void participantInterface()
{
    string userInput;
    string cmd = "";
    cout << "interfaceSubservice" << endl;
    cout << ">> ";
    while ((!stop_program) && (cmd != "EXIT") && (std::getline(std::cin, userInput)))
    {
        cmd = userInput.substr(0, userInput.find(" "));
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        cout << ">> ";
    }
    participantExit();
}

void participantExit()
{
    cout << "participantExit: Participant EXIT request from user" << endl;

    packet *p = new packet();
    p->type = EXIT_REQ;
    p->payload = "exit request";
    p->timestamp = getTimestamp();
    p->sender_mac = getMacAddr();
    p->sender_hostname = getHostname();
    p->sender_ip = getIpAddr();
    sendPacket(managerIP, MANAGER_PORT, p);

    stop_program = true;
    exit(0);
}

int broadcastSubservice()
{
    int n = receiveBroadcast(PARTICIPANT_PORT);
    return 0;
}
