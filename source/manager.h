#ifndef MANAGER_H
#define MANAGER_H

int startManager();

int monitoringSubservice();
int discoverySubservice();
int interfaceSubservice();
int messagesReceiverOriginal();
int messagesReceiverMonitoring();

void managerExit();

void printParticipants();

void populateFakeParticipants();

#endif