#ifndef MACHINE_H
#define MACHINE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <array>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <netdb.h> // to use hostent

#include "packet.h"
#include "participant.h"

#define MANAGER_PORT 4000
#define PARTICIPANT_PORT 4001

#define MONITORING_INTERVAL 5
#define DISCOVERY_INTERVAL 10
#define ROUNDS_WITHOUT_ACTIVITY_THRESHOLD 1

using namespace std;

class Machine
{
public:
    Machine(bool is_manager = false);

    void Start();

protected:
    string ip;
    string mac;
    string hostname;
    bool is_manager;
    atomic<uint16_t> seqn;
    atomic<bool> running;
    bool debug_mode;

    packet *new_packet(packet_type type);

    string get_mac();
    string get_hostname();
    string get_ip();
    string exec(const char *cmd);

    void message_receiver(int from_port);
    virtual void process_message(packet *rcvd_packet);
    virtual void interface();

    int sendPacket(packet_type type, string to_ip, int to_port, bool broadcast);

private: // participant functions
    string manager_ip;
    void send_exit();
};

#endif // MACHINE_H