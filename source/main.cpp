#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip> // to use setw
#include "datatypes.h"
#include <map> // to use map
#include <thread>
#include <netdb.h> // to use hostent, etc.

#include "udp_comm.cpp"

using namespace std;

map<string, participant> participants_map; // hostname -> participant

int startParticipant();
int startManager();

int discoverySubservice();

void printParticipants();
void populateFakeParticipants();

int main(int argc, char *argv[])
{
    cout << ("Workstation Sleep Manager\n") << endl;

    bool isManager = false;
    if (argc > 1)
        isManager = (strcmp(argv[1], "manager") == 0);

    cout << ("Started") << endl;

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

// -----------------------------------------------
//                  PARTICIPANT
// -----------------------------------------------

int startParticipant()
{
    int n;
    udp_comm *server = new udp_comm();

    n = initServer(*server);
    if (n == -1)
    {
        cout << "Error initializing participant" << endl;
        return -1;
    }

    n = receive(*server);
    if (n != -1)
    {
        cout << server->buf << endl;
    }

    n = send(*server, "Hello from participant");
    if (n == -1)
    {
        cout << "Error sending message" << endl;
        return -1;
    }

    cout << "ending participant" << endl;

    return 0;
}

// -----------------------------------------------
//                      MANAGER
// -----------------------------------------------

int startManager()
{
    // discovery subservice
    thread discoveryThread(discoverySubservice);

    // TODO: monitoring subservice
    // TODO: management subservice
    // TODO: interface subservice

    cout << "Enter to stop" << endl;
    cin.get();

    discoveryThread.join();

    populateFakeParticipants();
    printParticipants();

    cout << ("Manager stopped") << endl;
    return 0;
}

int discoverySubservice()
{
    cout << "Started DiscoverySubservice" << endl;
    int n;
    udp_comm *server = new udp_comm();

    n = initServer(*server);
    if (n == -1)
    {
        cout << "Error initializing manager" << endl;
        return -1;
    }

    n = send(*server, "Hello from manager");
    if (n == -1)
    {
        cout << "Error sending message" << endl;
        return -1;
    }

    n = receive(*server);
    if (n != -1)
        cout << server->buf << endl;

    cout << "ending manager" << endl;

    return 0;
}

// -----------------------------------------------
//                      UTILITIES
// -----------------------------------------------

void printParticipants()
{
    cout << setw(15) << "PARTICIPANTS" << endl;
    cout << left << setw(12)
         << "Hostname"
         << left << setw(12)
         << "IP"
         << left << setw(20)
         << "MAC"
         << left << setw(10)
         << "Status"
         << endl;
    for (const auto &[h, p] : participants_map)
    {
        cout
            << left << setw(12)
            << h
            << left << setw(12)
            << p.ip
            << left << setw(20)
            << p.mac
            << left << setw(10)
            << StatusToString(p.status)
            << endl;
    }
}

void populateFakeParticipants()
{
    participant p1;
    p1.ip = "1.1.1.1";
    p1.mac = "01:01:01:01:01:01";
    p1.status = awake;
    participants_map["host1"] = p1;

    participant p2;
    p2.ip = "1.1.1.2";
    p2.mac = "02:02:02:02:02:02";
    p2.status = asleep;
    participants_map["host2"] = p2;

    participant p3;
    p3.ip = "1.1.1.3";
    p3.mac = "03:03:03:03:03:03";
    p3.status = asleep;
    participants_map["host3"] = p3;
}