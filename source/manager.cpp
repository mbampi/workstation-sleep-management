#include <iostream>
#include <iomanip> // to use setw
#include <cstdlib>
#include <unistd.h>
#include <thread>

#include "participant.h"
#include "manager.h"
#include "mgmt_ss.h"
#define PACKETS_LIMIT 10

int startManager()
{
    cout << "startManager: creating monitoring thread" << endl;
    thread monitoringThread(monitoringSubservice);

    cout << "startManager: creating discovery thread" << endl;
    thread discoveryThread(discoverySubservice);

    cout << "startManager: creating msg receiver thread" << endl;
    thread messagesReceiverThread(messagesReceiver);

    cout << "startManager: populating fake participants" << endl;
    populateFakeParticipants(); // DEBUG

    string x = IPToHostname("1.1.1.3");
    cout << x << endl;

    // interface subservice
    string userInput = "";
    do
    {
        cout << "startManager: printing participants" << endl;

        cout << ">> ";
        cin >> userInput;
    } while (userInput != "EXIT");

    // TODO: monitoring subservice
    // TODO: management subservice

    cout << ("Manager EXIT request from user") << endl;

    monitoringThread.join();
    discoveryThread.join();
    messagesReceiverThread.join();

    cout << ("Manager stopped") << endl;
    return 0;
}

int monitoringSubservice()
{
    cout << "Started MonitoringSubservice" << endl;
    uint16_t seq_num = 0;
    do
    {
        cout << endl;

        packet *p = new packet();
        p->type = MONITORING_REQ;
        p->_payload = "monitoring_service_msg";
        p->seqn = seq_num;

        cout << "monitoringSubservice: sending packet " << p->seqn << endl;
        int sent_bytes = broadcastPacket(p, PARTICIPANT_PORT);
        if (sent_bytes < 0)
        {
            cout << "Error sending broadcast" << endl;
            return -1;
        }
        cout << "monitoringSubservice: broadcasted msg " << seq_num << " to port " << PARTICIPANT_PORT << " with size " << sent_bytes << endl;

        seq_num++;
        sleep(6);   // wait for 6 seconds
    } while (true); // TODO: add condition to stop

    cout << "ending monitoring" << endl;
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
        if (response->sender_hostname == "NULL")
        {
            string hn = IPToHostname(response->sender_ip);
            incrementLostPackets(hn);
            if (compareLostPackets(hn, PACKETS_LIMIT))
            {
                if (getStatus(hn) != asleep)
                    changeParticipantStatus(hn, asleep);
            }
        }
        cout << "messagesReceiver: packet response. type=" << response->type << " | seqn=" << response->seqn
             << " | length=" << response->length << " | payload=" << response->_payload << endl;

        switch (response->type)
        {
        case DISCOVERY_RES:
        {
            cout << "Received DISCOVERY_RES" << endl;

            participant *p = new participant();
            p->hostname = response->sender_hostname;
            p->ip = response->sender_ip;
            p->mac = response->sender_mac;
            p->hostname = response->_payload;
            p->state = awake;

            addParticipant(p);

            break;
        }
        case MONITORING_RES:
        {
            cout << "Received MONITORING_RES" << endl;
            zeroLostPackets(response->_payload); // hostname está em payload
            if (getStatus(response->_payload) != awake)
                changeParticipantStatus(response->_payload, awake);
            break;
        }
        default:
        {
            cout << "messagesReceiver: Received UNKNOWN packet" << endl;
            break;
        }
        }

    } while (true);
}

void populateFakeParticipants()
{
    participant *p1 = new participant();
    p1->ip = "1.1.1.1";
    p1->mac = "01:01:01:01:01:01";
    p1->state = awake;
    p1->hostname = "host1";
    addParticipant(p1);

    participant *p2 = new participant();
    p2->ip = "1.1.1.2";
    p2->mac = "02:02:02:02:02:02";
    p2->state = asleep;
    p2->hostname = "host2";
    addParticipant(p2);

    participant *p3 = new participant();
    p3->ip = "1.1.1.3";
    p3->mac = "03:03:03:03:03:03";
    p3->state = asleep;
    p3->hostname = "host3";
    addParticipant(p3);
}
