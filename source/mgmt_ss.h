#ifndef MGMT_SS_H
#define MGMT_SS_H

#include <map>
#include <string>
#include <mutex>
#include <vector>
#include <iomanip> // to use setw
#include <atomic>

#include "participant.h"

using namespace std;

extern atomic_bool stop_program;
extern atomic_bool debug_mode;

status getStatus(string hostname);
bool compareLostPackets(string hostname, int n);
string IPToHostname(string ip);
void zeroLostPackets(string hostname);
void incrementLostPackets(string hostname);

void printParticipants();
void addParticipant(participant *p);
void removeParticipant(string hostname);
void changeParticipantStatus(string hostname, status s);
vector<participant> getParticipants();
void wakeupParticipant(string hostname);

#endif