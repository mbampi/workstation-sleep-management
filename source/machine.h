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
#include <mutex>
#include <netdb.h> // to use hostent
#include <map>
#include <unistd.h>

#include "datatypes.h"

#define MANAGER_PORT 7000
#define PARTICIPANT_PORT 7001

#define MONITORING_INTERVAL 3
#define DISCOVERY_INTERVAL 5

#define ROUNDS_WITHOUT_ACTIVITY_THRESHOLD 1
#define LIMIT_FOR_ELECTION 6
#define WAIT_LIMIT_FOR_MSG 3

using namespace std;

class Machine
{
public:
    Machine();

    void Start();

protected:
    string ip;
    string mac;
    string hostname;
    atomic<uint16_t> seqn;
    atomic<bool> running;
    bool debug_mode;

    bool is_manager;
    bool election_running;

    int next_id;
    bool in_election;
    int election_iter;
    int nro_participants;

    packet *newPacket(packet_type type);

    string getMac();
    string getHostname();
    string getIP();
    string getBroadcastIP();
    string exec(const char *cmd);

    int newReceiverSocket();
    void messageReceiver();

    void processMessage(packet *rcvd_packet);
    void interface();
    void setSelfAsManager();

    int sendPacket(packet_type type, string to_ip, int to_port, bool broadcast);

    // participant
    string manager_ip; // participant_attribute
    void participantSendExit();
    void processMessageAsParticipant(packet *rcvd_packet);
    void sendExit();

    // manager
    map<string, participant_info> participantsMap; // hostname -> participant
    mutex participantsMapMutex;

    void processMessageAsManager(packet *rcvd_packet);

    void printParticipants();
    void setParticipantsMap(vector<participant_info> new_participants);
    void addParticipant(participant_info *p);
    void removeParticipant(string hostname);
    void changeParticipantStatus(string hostname, status s);
    vector<participant_info> getParticipants();
    void zeroRoundsWithoutActivity(string hostname);
    void incRoundsWithoutActivity(string hostname);

    string encodeParticipants();
    vector<participant_info> decodeParticipants(string participants_str);
    void sendParticipantsReplicaToAll();

    void discovery();
    void monitoring();
    void statusRecognizer();

    void sendWakeOnLan(string mac);
    void wakeupParticipant(string hostname);

    void election();
};

#endif // MACHINE_H
