#include "mgmt_ss.h"
#include "participant.h"

map<string, participant> participants_map; // hostname -> participant
mutex participants_map_mutex;

void addParticipant(participant p)
{
    participants_map_mutex.lock();
    participants_map[p.hostname] = p;
    participants_map_mutex.unlock();
}

void removeParticipant(string hostname)
{
    participants_map_mutex.lock();
    participants_map.erase(hostname);
    participants_map_mutex.unlock();
}

void changeParticipantStatus(string hostname, status s)
{
    participants_map_mutex.lock();
    participants_map[hostname].status = s;
    participants_map_mutex.unlock();
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
