#ifndef MANAGER_H
#define MANAGER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <netdb.h>  // to use hostent
#include <unistd.h> // sleep
#include <iomanip>  // to use setw
#include <map>
#include <vector>
#include <mutex>

#include "machine.h"
#include "datatypes.h"

using namespace std;

class Manager : public Machine
{
public:
    void Start();

protected:
    map<string, participant_info> participants_map; // hostname -> participant
    mutex participants_map_mutex;

    void printParticipants();
    void addParticipant(participant_info *p);
    void removeParticipant(string hostname);
    void changeParticipantStatus(string hostname, status s);
    vector<participant_info> getParticipants();
    void zero_rounds_without_activity(string hostname);
    void inc_rounds_without_activity(string hostname);

    void discovery();
    void monitoring();
    void process_message(packet *rcvd_packet) override;
    void interface() override;

    void sendWakeOnLan(string mac);
    void wakeupParticipant(string hostname);
};

#endif