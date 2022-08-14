#ifndef MANAGER_H
#define MANAGER_H

int startManager();

int monitoringSubservice();
int discoverySubservice();
int messagesReceiver();

void printParticipants();

void populateFakeParticipants();

#endif