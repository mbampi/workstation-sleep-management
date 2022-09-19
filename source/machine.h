#ifndef MACHINE_H
#define MACHINE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <array>
#include <atomic>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <netdb.h> // to use hostent
#include <map>

#include "datatypes.h"

#define MANAGER_PORT 7000
#define PARTICIPANT_PORT 7001

#define MONITORING_INTERVAL 5
#define DISCOVERY_INTERVAL 10
#define ROUNDS_WITHOUT_ACTIVITY_THRESHOLD 1

using namespace std;

class Machine
{
public:
    Machine(bool is_manager);

    void Start();

protected:
    string ip;
    string mac;
    string hostname;
    atomic<uint16_t> seqn;
    atomic<bool> running;
    bool debug_mode;

    bool is_manager;

    packet *newPacket(packet_type type);

    string getMac();
    string getHostname();
    string getIP();
    string getBroadcastIP();
    string exec(const char *cmd);

    void messageReceiver(int from_port);

    void processMessage(packet *rcvd_packet);
    void interface();

    int sendPacket(packet_type type, string to_ip, int to_port, bool broadcast);

    // participant
    string manager_ip; // participant_attribute
    void participantStart();
    void participantSendExit();
    void processMessageAsParticipant(packet *rcvd_packet);
    void participantInterface();
    void sendExit();

    // manager
    map<string, participant_info> participantsMap; // hostname -> participant
    mutex participantsMapMutex;

    void managerStart();
    void processMessageAsManager(packet *rcvd_packet);
    void managerInterface();

    void printParticipants();
    void addParticipant(participant_info *p);
    void removeParticipant(string hostname);
    void changeParticipantStatus(string hostname, status s);
    vector<participant_info> getParticipants();
    void zeroRoundsWithoutActivity(string hostname);
    void incRoundsWithoutActivity(string hostname);

    void discovery();
    void monitoring();

    void sendWakeOnLan(string mac);
    void wakeupParticipant(string hostname);
};

#endif // MACHINE_H