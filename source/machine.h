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

    packet *new_packet(packet_type type);

    string get_mac();
    string get_hostname();
    string get_ip();
    string get_broadcast_ip();
    string exec(const char *cmd);

    void message_receiver(int from_port);

    void process_message(packet *rcvd_packet);
    void interface();

    int sendPacket(packet_type type, string to_ip, int to_port, bool broadcast);

    // participant
    string manager_ip; // participant_attribute
    void participant_start();
    void participant_send_exit();
    void process_message_as_participant(packet *rcvd_packet);
    void participant_interface();
    void send_exit();

    // manager
    map<string, participant_info> participants_map; // hostname -> participant
    mutex participants_map_mutex;

    void manager_start();
    void process_message_as_manager(packet *rcvd_packet);
    void manager_interface();

    void printParticipants();
    void addParticipant(participant_info *p);
    void removeParticipant(string hostname);
    void changeParticipantStatus(string hostname, status s);
    vector<participant_info> getParticipants();
    void zero_rounds_without_activity(string hostname);
    void inc_rounds_without_activity(string hostname);

    void discovery();
    void monitoring();

    void sendWakeOnLan(string mac);
    void wakeupParticipant(string hostname);
};

#endif // MACHINE_H