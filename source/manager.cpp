#include <iostream>
#include <iomanip> // to use setw
#include <cstdlib>
#include <unistd.h>
#include <thread>

#include "manager.h"
#include "participant.h"

map<string, participant> participants_map; // hostname -> participant

int startManager()
{
    thread discoveryThread(discoverySubservice);

    thread messagesReceiverThread(messagesReceiver);

    populateFakeParticipants(); // DEBUG

    // interface subservice
    string userInput = "";
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
    messagesReceiverThread.join();

    cout << ("Manager stopped") << endl;
    return 0;
}

int discoverySubservice()
{
    cout << "Started DiscoverySubservice" << endl;
    uint16_t seq_num = 0;
    do
    {
        cout << endl;

        packet *p = new packet();
        p->type = DISCOVERY_REQ;
        p->_payload = "discovery_service_msg";
        p->seqn = seq_num;

        int sent_bytes = broadcastPacket(p, PARTICIPANT_PORT);
        if (sent_bytes < 0)
        {
            cout << "Error sending broadcast" << endl;
            return -1;
        }
        cout << "DEBUG: broadcasted msg " << seq_num << " to port " << PARTICIPANT_PORT << " with size " << sent_bytes << endl;

        seq_num++;
        sleep(6);   // wait for 6 seconds
    } while (true); // TODO: add condition to stop

    cout << "ending discovery" << endl;
    return 0;
}

int messagesReceiver()
{
    cout << "Started MessagesReceiver" << endl;
    do
    {
        cout << endl;

        // receive response
        cout << "Waiting for message on port " << MANAGER_PORT << endl;
        packet *response = receivePacket(MANAGER_PORT);
        if (response == NULL)
        {
            cout << "Error receiving packet" << endl;
            return -1;
        }
        cout << "DEBUG: packet response type=" << response->type << " | seqn: " << response->seqn << " | length: " << response->length << " | payload:" << response->_payload << endl;
    } while (true);
}

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
            << p.status // StatusToString(
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
