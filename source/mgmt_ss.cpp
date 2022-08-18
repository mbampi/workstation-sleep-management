#include "mgmt_ss.h"
#include "participant.h"

using namespace std;

map<string, participant> participants_map; // hostname -> participant
mutex participants_map_mutex;

atomic_bool stop_program = {false};
atomic_bool debug_mode = {false};

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

    for (auto const &p : participants_map)
    {
        if (p.second.ip == ip)
            hostname = p.first;
    }
    participants_map_mutex.unlock();

    return hostname;
}

void zeroLostPackets(string hostname)
{
    if (isParticipant(hostname))
    {
        participants_map_mutex.lock();
        participants_map[hostname].lost_packets = 0;
        participants_map_mutex.unlock();
    }
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
    cout << left << setw(25)
         << "Hostname"
         << left << setw(25)
         << "IP"
         << left << setw(25)
         << "MAC"
         << left << setw(10)
         << "Status"
         << endl;
    for (const participant p : getParticipants())
    {
        cout
            << left << setw(25)
            << p.hostname
            << left << setw(25)
            << p.ip
            << left << setw(25)
            << p.mac
            << left << setw(10)
            << StatusToString(p.state)
            << endl;
    }
}

void addParticipant(participant *p)
{
    if (debug_mode)
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
    for (auto const &p : participants_map)
    {
        participants.push_back(p.second);
    }
    participants_map_mutex.unlock();
    return participants;
}

void wakeupParticipant(string hostname)
{
    if (debug_mode)
        cout << "wakeupParticipant: waking up " << hostname << endl;
    auto p = participants_map[hostname];
    sendWakeOnLan(p.mac);
}

bool isParticipant(string hostname)
{
    bool is_participant = false;
    vector<participant> p_list = getParticipants();

    for (size_t i = 0; i < p_list.size(); i++)
    {
        if (p_list[i].hostname == hostname)
            is_participant = true;
    }

    return is_participant;
}
