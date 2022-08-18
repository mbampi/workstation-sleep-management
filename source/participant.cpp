
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
    if (debug_mode)
        cout << "Started Participant" << endl;

    thread broadcastReceiverThread(broadcastSubservice);
    thread singleReceiverThread(singleSubservice);

    participantInterface();

    broadcastReceiverThread.join();
    singleReceiverThread.join();

    if (debug_mode)
        cout << "ending participant" << endl;

    return 0;
}

void participantInterface()
{
    string userInput;
    string cmd = "";
    if (debug_mode)
        cout << "interfaceSubservice" << endl;
    cout << ">> ";    

    while ((!stop_program) && (cmd != "EXIT") && (getline(cin, userInput)))
    {
        cmd = userInput.substr(0, userInput.find(" "));
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        cout << ">> ";
    }
    participantExit();
}

void participantExit()
{
    if (debug_mode)
        cout << "participantExit: Participant EXIT request from user" << endl;

    packet *p = new packet();
    p->type = EXIT_REQ;
    p->payload = "exit request";
    p->timestamp = getTimestamp();
    p->sender_mac = getMacAddr();
    p->sender_hostname = getHostname();
    p->sender_ip = getIpAddr();
    sendPacket(managerIP, MANAGER_DISCOVERY_PORT, p);

    stop_program = true;
    exit(0);
}

int broadcastSubservice()
{
    int n = receiveBroadcast(PARTICIPANT_DISCOVERY_PORT);
    return 0;
}

int singleSubservice() //deixei no receive broadcast e não no receive packet por enquanto para teste
{
    //packet *p = receivePacket(PARTICIPANT_MONITORING_PORT);
    int n = receiveBroadcast(PARTICIPANT_MONITORING_PORT);

    return 0;
}