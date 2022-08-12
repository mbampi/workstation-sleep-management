

#ifndef UDP_COMM_H
#define UDP_COMM_H

#include <string.h>
#include "packet.h"

#define PARTICIPANT_PORT 4001
#define MANAGER_PORT 4000

int sendPacket(char *ip, int port, packet *p);

packet *receivePacket(int on_port);

int broadcastMessage(string msg, int port);

int broadcastPacket(packet *msg, int port);

int receiveBroadcast(int on_port);

#endif
