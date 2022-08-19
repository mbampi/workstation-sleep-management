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

#include "machine.h"
#include "packet.h"
#include "participant.h"

using namespace std;

class Manager : private Machine
{
public:
    Manager();

    void Start();

protected:
    map<string, participant> participants_map; // hostname -> participant
    mutex participants_map_mutex;

    void printParticipants();
    void addParticipant(participant *p);
    void removeParticipant(string hostname);
    void changeParticipantStatus(string hostname, status s);
    vector<participant> getParticipants();
    void zero_rounds_without_activity(string hostname);
    void inc_rounds_without_activity(string hostname);

    void discovery();
    void monitoring();
    void process_message(packet *rcvd_packet) override;
    void interface() override;

    void sendWakeOnLan(string mac);
    void wakeupParticipant(string hostname);
};
