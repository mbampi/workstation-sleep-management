

#ifndef UDP_COMM_H
#define UDP_COMM_H

#include <string.h>
#include "packet.h"

#define PARTICIPANT_PORT 4001
#define MANAGER_PORT 4000

extern atomic_uint16_t seqn;

int sendPacket(char *ip, int port, packet *p);

packet *receivePacket(int on_port);

int broadcastPacket(packet *msg, int port);

int receiveBroadcast(int on_port);

string getMacAddr();
string getHostname();
string getIpAddr();
uint16_t getTimestamp();
void sendWakeOnLan(string mac);

#endif
