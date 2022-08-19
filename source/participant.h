#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>

#include "machine.h"
#include "datatypes.h"

using namespace std;

class Participant : public Machine
{
public:
    void Start();

protected:
    string manager_ip;
    void send_exit();
    void process_message(packet *rcvd_packet) override;
    void interface() override;
};

#endif // PARTICIPANT_H