
#ifndef MANAGER_H
#define MANAGER_H

#include <map>
#include <string>

#include "participant.h"

int startManager();

int discoverySubservice();
int messagesReceiver();

void printParticipants();

void populateFakeParticipants();

#endif