#include <iostream>
#include <iomanip> // to use setw
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <algorithm>

#include "participant.h"
#include "manager.h"
#include "mgmt_ss.h"
#define PACKETS_LIMIT 10

int startManager()
{
    cout << "startManager: creating discoverySubservice thread" << endl;
    thread discoveryThread(discoverySubservice);

    cout << "startManager: creating monitoringSubservice thread" << endl;
    thread monitoringThread(monitoringSubservice);

    cout << "startManager: creating messagesReceiver thread" << endl;
    thread messagesReceiverThread(messagesReceiver);

    cout << "startManager: populating fake participants" << endl;
    populateFakeParticipants(); // DEBUG

    // interface subservice
    cout << "startManager: running interfaceSubservice" << endl;
    interfaceSubservice();

    cout << ("startManager: Manager EXIT request from user") << endl;

    discoveryThread.join();
    monitoringThread.join();
    messagesReceiverThread.join();

    cout << ("startManager: Manager stopped") << endl;
    return 0;
}

int interfaceSubservice()
{
    string userInput;
    string cmd = "";
    cout << "interfaceSubservice" << endl;
    cout << ">> ";
    while ((!stop_program) && (cmd != "EXIT") && (getline(cin, userInput)))
    {
        cmd = userInput.substr(0, userInput.find(" "));
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if (cmd == "EXIT")
        {
            cout << ("startManager: Manager EXIT request from user") << endl;
        }
        else if (cmd == "HELP")
        {
            cout << "startManager: printing help" << endl;
            cout << "HELP: print this help" << endl;
            cout << "EXIT: stop the manager" << endl;
            cout << "LIST: print the list of participants" << endl;
            cout << "WAKEUP <hostname>: wake the participant with the given hostname" << endl;
        }
        else if (cmd == "LIST")
        {
            cout << "startManager: printing participants" << endl;
            printParticipants();
        }
        else if (cmd == "WAKEUP")
        {
            string hostname = userInput.substr(userInput.find(" ") + 1);
            cout << "startManager: waking up participant" << hostname << endl;
            wakeupParticipant(hostname);
        }
        else
        {
            cout << "startManager: invalid command" << endl;
        }
        cout << ">> ";
    };

    managerExit();
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
        p->payload = "monitoring_service_msg";
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
        sleep(6);            // wait for 6 seconds
    } while (!stop_program); // TODO: add condition to stop

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
        p->payload = "discovery_service_msg";
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
        sleep(6); // wait for 6 seconds
    } while (!stop_program);

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
             << " | length=" << response->length << " | payload=" << response->payload << endl;

        switch (response->type)
        {
        case DISCOVERY_RES:
        {
            cout << "Received DISCOVERY_RES" << endl;

            participant *p = new participant();
            p->ip = response->sender_ip;
            p->mac = response->sender_mac;
            p->hostname = response->sender_hostname;
            p->state = awake;

            addParticipant(p);
            break;
        }
        case MONITORING_RES:
        {
            cout << "Received MONITORING_RES" << endl;
            zeroLostPackets(response->payload); // hostname está em payload
            if (getStatus(response->payload) != awake)
                changeParticipantStatus(response->payload, awake);
            break;
        }
        case EXIT_REQ:
        {
            cout << "Received EXIT_REQ" << endl;
            removeParticipant(response->sender_hostname);
            break;
        }
        default:
        {
            cout << "messagesReceiver: Received UNKNOWN packet" << endl;
            break;
        }
        }
    } while (!stop_program);
    return 0;
}

void managerExit()
{
    cout << "managerExit" << endl;
    stop_program = true;
    exit(0);
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