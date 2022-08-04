#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip> // to use setw
#include <map>     // to use map
#include <thread>
#include <netdb.h> // to use hostent

#include "udp_comm.cpp"
#include "participant.cpp"
#include "packet.cpp"

using namespace std;

#define PARTICIPANT_PORT 4001
#define MANAGER_PORT 4000

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
    // udp_comm *server = new udp_comm();
    int n = receiveBroadcast();
    cout << "Received broadcast " << n << endl;

    // n = initServer(*server, PARTICIPANT_PORT);
    // if (n == -1)
    // {
    //     cout << "Error initializing participant" << endl;
    //     return -1;
    // }

    // while (true)
    // {
    //     n = receive(*server);
    //     if (n != -1)
    //     {
    //         cout << server->buf << endl;
    //     }

    //     n = send(*server, "Hello from participant", MANAGER_PORT);
    //     if (n == -1)
    //     {
    //         cout << "Error sending message" << endl;
    //         return -1;
    //     }
    // }

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

    // interface subservice
    string userInput = "";
    do
    {
        printParticipants();
        populateFakeParticipants(); // testing
        cout << "broadcasting" << endl;
        broadcastMessage("Hello from manager");

        cout << ">> ";
        cin >> userInput;
    } while (userInput != "EXIT");

    // TODO: monitoring subservice
    // TODO: management subservice

    discoveryThread.join();

    cout << ("Manager stopped") << endl;
    return 0;
}

int discoverySubservice()
{
    cout << "Started DiscoverySubservice" << endl;
    int n;
    udp_comm *server = new udp_comm();

    n = initServer(*server, MANAGER_PORT);
    if (n == -1)
    {
        cout << "Error initializing manager" << endl;
        return -1;
    }

    // n = send(*server, "Hello from manager", PARTICIPANT_PORT);
    // if (n == -1)
    // {
    //     cout << "Error sending message" << endl;
    //     return -1;
    // }
    while (true)
    {
        // keeps listening to participants
        n = receive(*server);
        if (n != -1)
            cout << server->buf << endl;
    }

    cout << "ending discovery" << endl;

    return 0;
}

// -----------------------------------------------
//                  UTILITIES
// -----------------------------------------------

void printParticipants()
{
    cout << setw(15) << "\nPARTICIPANTS" << endl;
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
