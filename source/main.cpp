#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip> // to use setw
#include <map>     // to use map
#include <thread>
#include <netdb.h> // to use hostent

#include "packet.cpp"
#include "udp_comm.cpp"
#include "participant.cpp"

#include <cstdlib>
#include <unistd.h>

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
    int n = receiveBroadcast(PARTICIPANT_PORT);
    cout << "Received broadcast " << n << endl;

    cout << "ending participant" << endl;

    return 0;
}

// -----------------------------------------------
//                      MANAGER
// -----------------------------------------------

int startManager()
{
    thread discoveryThread(discoverySubservice);

    populateFakeParticipants(); // DEBUG

    // interface subservice
    string userInput = "msg 1";
    do
    {
        printParticipants();

        cout << ">> ";
        cin >> userInput;
    } while (userInput != "EXIT");

    // TODO: monitoring subservice
    // TODO: management subservice

    cout << ("Manager EXIT request from user") << endl;

    discoveryThread.join();

    cout << ("Manager stopped") << endl;
    return 0;
}

int discoverySubservice()
{
    cout << "Started DiscoverySubservice" << endl;
    uint16_t seq_num = 1;
    do
    {
        bool string_msg = false;
        if (string_msg)
        {
            broadcastMessage("discovery_service_msg", PARTICIPANT_PORT);
        }
        else
        {
            packet *p = new packet();
            p->type = packet_type::DISCOVERY;
            p->_payload = "discovery_service_msg";
            p->length = sizeof("discovery_service_msg");
            p->seqn = seq_num;
            seq_num++;
            int sent_bytes = broadcastPacket(p, PARTICIPANT_PORT);
            if (sent_bytes < 0)
            {
                cout << "Error sending broadcast" << endl;
                return -1;
            }
            cout << "DEBUG: broadcasted msg " << seq_num << " to port " << PARTICIPANT_PORT << " with size " << sent_bytes << endl;
        }
        sleep(2);   // wait for 2 seconds
    } while (true); // TODO: add condition to stop

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
