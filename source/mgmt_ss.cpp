#include "mgmt_ss.h"
#include "participant.h"

map<string, participant> participants_map; // hostname -> participant
mutex participants_map_mutex;

std::atomic_bool stop_program = {false};

status getStatus(string hostname)
{
    participants_map_mutex.lock();
    status state = participants_map[hostname].state;
    participants_map_mutex.unlock();

    return state;
}

bool compareLostPackets(string hostname, int n)
{
    participants_map_mutex.lock();
    bool cmp = (participants_map[hostname].lost_packets >= n);
    participants_map_mutex.unlock();

    return cmp;
}

string IPToHostname(string ip)
{
    string hostname;

    participants_map_mutex.lock();
    for (const auto &[h, p] : participants_map)
    {
        if (p.ip == ip)
        {
            hostname = p.hostname;
        }
    }
    participants_map_mutex.unlock();

    return hostname;
}

void zeroLostPackets(string hostname)
{
    participants_map_mutex.lock();
    participants_map[hostname].lost_packets = 0;
    participants_map_mutex.unlock();
}

void incrementLostPackets(string hostname)
{
    participants_map_mutex.lock();
    participants_map[hostname].lost_packets += 1;
    participants_map_mutex.unlock();
}

void printParticipants()
{
    cout << setw(15) << "\nPARTICIPANTS" << endl;
    cout << left << setw(20)
         << "Hostname"
         << left << setw(20)
         << "IP"
         << left << setw(20)
         << "MAC"
         << left << setw(10)
         << "Status"
         << endl;
    for (const participant p : getParticipants())
    {
        cout
            << left << setw(20)
            << p.hostname
            << left << setw(20)
            << p.ip
            << left << setw(20)
            << p.mac
            << left << setw(10)
            << StatusToString(p.state)
            << endl;
    }
}

void addParticipant(participant *p)
{
    cout << "\naddParticipant: adding participant " << p->hostname << endl;
    participants_map_mutex.lock();
    participants_map[p->hostname] = *p;
    participants_map_mutex.unlock();
    printParticipants();
}

void removeParticipant(string hostname)
{
    participants_map_mutex.lock();
    participants_map.erase(hostname);
    participants_map_mutex.unlock();
    printParticipants();
}

void changeParticipantStatus(string hostname, status s)
{
    participants_map_mutex.lock();
    participants_map[hostname].state = s;
    participants_map_mutex.unlock();
    printParticipants();
}

vector<participant> getParticipants()
{
    vector<participant> participants;
    participants_map_mutex.lock();
    for (const auto &[h, p] : participants_map)
    {
        participants.push_back(p);
    }
    participants_map_mutex.unlock();
    return participants;
}

void wakeupParticipant(string hostname)
{
    cout << "wakeupParticipant: waking up " << hostname << endl;
    participant p = participants_map[hostname];
    // packet *p = new packet();
    // sendPacket(p.ip, PARTICIPANT_PORT, "wakeup");
}
