

#ifndef UDP_COMM_H
#define UDP_COMM_H

#include <string.h>
#include "packet.h"
#include <atomic>

#define MANAGER_DISCOVERY_PORT 4000
#define PARTICIPANT_DISCOVERY_PORT 4001
#define MANAGER_MONITORING_PORT  5000
#define PARTICIPANT_MONITORING_PORT 5001

extern atomic<uint16_t> seqn;

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
