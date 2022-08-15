
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <thread>

#include "packet.h"
#include "udp_comm.h"
#include "mgmt_ss.h"
#include "participant.h"

using namespace std;

char *managerIP;

participant *decode_participantpayload(string payload)
{
    participant *p = new participant();
    char *token = strtok((char *)payload.c_str(), ";");
    p->hostname = token;
    token = strtok(NULL, ";");
    p->mac = token;
    return p;
}

string encode_participantpayload()
{
    string hostname = getHostname();
    string mac = getMacAddr();
    string payload = hostname + ";" + mac + ";";
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

    packet *exitPacket = new packet();
    exitPacket->type = EXIT_REQ;
    exitPacket->seqn = 0;
    exitPacket->payload = "exit request";
    sendPacket(managerIP, MANAGER_PORT, exitPacket);

    stop_program = true;
    exit(0);
}

int broadcastSubservice()
{
    int n = receiveBroadcast(PARTICIPANT_PORT);
    return 0;
}
