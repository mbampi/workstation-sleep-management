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
    if (debug_mode)
        cout << "startManager: creating discoverySubservice thread" << endl;
    thread discoveryThread(discoverySubservice);

    if (debug_mode)
        cout << "startManager: creating monitoringSubservice thread" << endl;
    thread monitoringThread(monitoringSubservice);

    if (debug_mode)
        cout << "startManager: creating messagesReceiver thread" << endl;
    thread messagesReceiverThread(messagesReceiver);

    if (debug_mode)
        cout << "startManager: populating fake participants" << endl;
    // populateFakeParticipants(); // DEBUG

    if (debug_mode)
        cout << "startManager: running interfaceSubservice" << endl;
    interfaceSubservice();

    if (debug_mode)
        cout << ("startManager: Manager EXIT request from user") << endl;

    discoveryThread.join();
    monitoringThread.join();
    messagesReceiverThread.join();

    if (debug_mode)
        cout << ("startManager: Manager stopped") << endl;
    return 0;
}

int interfaceSubservice()
{
    string userInput;
    string cmd = "";
    if (debug_mode)
        cout << "interfaceSubservice" << endl;
    cout << ">> ";
    while ((!stop_program) && (cmd != "EXIT") && (getline(cin, userInput)))
    {
        cmd = userInput.substr(0, userInput.find(" "));
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if (cmd == "EXIT")
        {
            if (debug_mode)
                cout << ("startManager: Manager EXIT request from user") << endl;
        }
        else if (cmd == "HELP")
        {
            if (debug_mode)
                cout << "startManager: printing help" << endl;
            cout << endl;
            cout << "HELP: print this help" << endl;
            cout << "EXIT: stop the manager" << endl;
            cout << "LIST: print the list of participants" << endl;
            cout << "WAKEUP <hostname>: wake the participant with the given hostname" << endl;
            cout << endl;
        }
        else if (cmd == "LIST")
        {
            if (debug_mode)
                cout << "startManager: printing participants" << endl;
            printParticipants();
        }
        else if (cmd == "WAKEUP")
        {
            string hostname = userInput.substr(userInput.find(" ") + 1);
            if (debug_mode)
                cout << "startManager: waking up participant " << hostname << endl;
            wakeupParticipant(hostname);
        }
        else
        {
            cout << "Invalid command" << endl;
        }
        cout << ">> ";
    };

    managerExit();
    return 0;
}

int monitoringSubservice()
{
    if (debug_mode)
        cout << "Started MonitoringSubservice" << endl;
    do
    {
        if (debug_mode)
            cout << endl;

        packet *p = new packet();
        p->type = MONITORING_REQ;
        p->payload = "monitoring_service_msg";
        p->timestamp = getTimestamp();
        p->sender_mac = getMacAddr();
        p->sender_hostname = getHostname();
        p->sender_ip = getIpAddr();

        if (debug_mode)
            cout << "monitoringSubservice: sending packet " << p->seqn << endl;
        for (const participant part : getParticipants())
        {
            int sent_bytes = sendPacket((char *)part.ip.c_str(), PARTICIPANT_PORT, p);
            if (sent_bytes < 0)
            {
                cout << "Error sending broadcast to participant " << part.hostname << endl;
                return -1;
            }
            if (debug_mode)
                cout << "monitoringSubservice: sent msg to " << part.hostname << " with size " << sent_bytes << endl;
        }
        sleep(6);            // wait for 6 seconds
    } while (!stop_program); // TODO: add condition to stop

    if (debug_mode)
        cout << "ending monitoring" << endl;
    return 0;
}

int discoverySubservice()
{
    if (debug_mode)
        cout << "Started DiscoverySubservice" << endl;
    do
    {
        if (debug_mode)
            cout << endl;

        packet *p = new packet();
        p->type = DISCOVERY_REQ;
        p->payload = "discovery_service_msg";
        p->timestamp = getTimestamp();
        p->sender_mac = getMacAddr();
        p->sender_hostname = getHostname();
        p->sender_ip = getIpAddr();

        if (debug_mode)
            cout << "discoverySubservice: sending packet " << p->seqn << endl;

        int sent_bytes = broadcastPacket(p, PARTICIPANT_PORT);
        if (sent_bytes < 0)
        {
            cout << "Error sending broadcast" << endl;
            return -1;
        }
        if (debug_mode)
            cout << "discoverySubservice: broadcasted msg to port " << PARTICIPANT_PORT << " with size " << sent_bytes << endl;
        sleep(6); // wait for 6 seconds
    } while (!stop_program);
    if (debug_mode)
        cout << "ending discovery" << endl;
    return 0;
}

int messagesReceiver()
{
    if (debug_mode)
        cout << "Started MessagesReceiver" << endl;
    do
    {
        if (debug_mode)
            cout << endl;

        // receive response
        if (debug_mode)
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
        if (debug_mode)
            cout << "messagesReceiver: packet response. type=" << response->type << " | seqn=" << response->seqn
                 << " | length=" << response->length << " | payload=" << response->payload << endl;

        switch (response->type)
        {
        case DISCOVERY_RES:
        {
            if (debug_mode)
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
            if (debug_mode)
                cout << "Received MONITORING_RES" << endl;
            zeroLostPackets(response->payload); // hostname está em payload
            if (getStatus(response->payload) != awake)
                changeParticipantStatus(response->payload, awake);
            break;
        }
        case EXIT_REQ:
        {
            if (debug_mode)
                cout << "Received EXIT_REQ" << endl;
            removeParticipant(response->sender_hostname);
            break;
        }
        default:
        {
            if (debug_mode)
                cout << "messagesReceiver: Received UNKNOWN packet" << endl;
            break;
        }
        }
    } while (!stop_program);
    return 0;
}

void managerExit()
{
    if (debug_mode)
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