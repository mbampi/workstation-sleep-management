#include <iostream>
#include <iomanip> // to use setw
#include <cstdlib>
#include <unistd.h>
#include <thread>

#include "participant.h"
#include "manager.h"
#include "mgmt_ss.h"

int startManager()
{
    cout << "startManager: creating discovery thread" << endl;
    thread discoveryThread(discoverySubservice);

    cout << "startManager: creating msg receiver thread" << endl;
    thread messagesReceiverThread(messagesReceiver);

    cout << "startManager: populating fake participants" << endl;
    populateFakeParticipants(); // DEBUG

    // interface subservice
    string userInput = "";
    do
    {
        cout << "startManager: printing participants" << endl;
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

        cout << "discoverySubservice: sending packet " << p->seqn << endl;
        int sent_bytes = broadcastPacket(p, PARTICIPANT_PORT);
        if (sent_bytes < 0)
        {
            cout << "Error sending broadcast" << endl;
            return -1;
        }
        cout << "discoverySubservice: broadcasted msg " << seq_num << " to port " << PARTICIPANT_PORT << " with size " << sent_bytes << endl;

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
        cout << "messagesReceiver: Waiting for message on port " << MANAGER_PORT << endl;
        auto response = receivePacket(MANAGER_PORT);
        if (response == NULL)
        {
            cout << "messagesReceiver: Error receiving packet" << endl;
            return -1;
        }
        cout << "messagesReceiver: packet response. type=" << response->type << " | seqn=" << response->seqn
             << " | length=" << response->length << " | payload=" << response->_payload << endl;

        if (response->type == DISCOVERY_RES)
        {
            cout << "Received DISCOVERY_RES" << endl;

            participant *p = new participant();
            p->hostname = response->sender_hostname;
            p->ip = response->sender_ip;
            p->mac = response->sender_mac;
            p->hostname = response->_payload;
            p->status = awake;

            addParticipant(p);
        }
        else
        {
            cout << "messagesReceiver: Received UNKNOWN packet" << endl;
        }

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
    for (const participant p : getParticipants())
    {
        cout
            << left << setw(12)
            << p.hostname
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
    participant *p1 = new participant();
    p1->ip = "1.1.1.1";
    p1->mac = "01:01:01:01:01:01";
    p1->status = awake;
    p1->hostname = "host1";
    addParticipant(p1);

    participant *p2 = new participant();
    p2->ip = "1.1.1.2";
    p2->mac = "02:02:02:02:02:02";
    p2->status = asleep;
    p2->hostname = "host2";
    addParticipant(p2);

    participant *p3 = new participant();
    p3->ip = "1.1.1.3";
    p3->mac = "03:03:03:03:03:03";
    p3->status = asleep;
    p3->hostname = "host3";
    addParticipant(p3);
}
