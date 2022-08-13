#ifndef MGMT_SS_H
#define MGMT_SS_H

#include <map>
#include <string>
#include <mutex>
#include <vector>

#include "participant.h"

using namespace std;

void addParticipant(participant *p);
void removeParticipant(string hostname);
void changeParticipantStatus(string hostname, status s);
vector<participant> getParticipants();

#endif