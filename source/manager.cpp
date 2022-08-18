
#include "manager.h"

Manager::Manager()
{
    Machine(true);
}

void Manager::Start()
{
    thread message_receiver_thread([this]
                                   { this->message_receiver(MANAGER_PORT); });
    thread discovery_thread([this]
                            { this->discovery(); });
    this->monitoring();

    discovery_thread.join();
    message_receiver_thread.join();
}

void Manager::monitoring()
{
    do
    {
        cout << "manager: monitoring: sending req" << endl;

        for (auto &p : this->getParticipants())
        {
            int sent_bytes = this->sendPacket(MONITORING_REQ, p.ip, PARTICIPANT_PORT, false);
            cout << "manager: montitoring: sent_bytes=" << sent_bytes << endl;
        }

        sleep(8);
    } while (true);
}

void Manager::discovery()
{
    do
    {
        cout << "manager: discovery: sending req" << endl;
        this->sendPacket(DISCOVERY_REQ, "", PARTICIPANT_PORT, true);
        sleep(10);
    } while (true);
}

void Manager::process_message(packet *rcvd_packet)
{
    cout << "manager: process_message: received packet" << endl;
    switch (rcvd_packet->type)
    {
    case DISCOVERY_RES:
    {
        if (debug_mode)
            cout << "process_message: Received DISCOVERY_RES" << endl;

        participant *p = new participant();
        p->ip = rcvd_packet->sender_ip;
        p->mac = rcvd_packet->sender_mac;
        p->hostname = rcvd_packet->sender_hostname;
        p->state = awake;

        this->addParticipant(p);

        break;
    }
    case MONITORING_RES:
    {
        if (debug_mode)
            cout << "process_message: Received MONITORING_RES" << endl;

        break;
    }
    case EXIT_REQ:
    {
        if (debug_mode)
            cout << "process_message: Received EXIT_REQ" << endl;
        removeParticipant(rcvd_packet->sender_hostname);
        break;
    }
    default:
    {
        if (debug_mode)
            cout << "process_message: Received UNKNOWN packet" << endl;
        break;
    }
    }
}

void Manager::printParticipants()
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
            << status_to_string(p.state)
            << endl;
    }
}

void Manager::addParticipant(participant *p)
{
    if (debug_mode)
        cout << "\naddParticipant: adding participant " << p->hostname << endl;
    participants_map_mutex.lock();
    participants_map[p->hostname] = *p;
    participants_map_mutex.unlock();
    printParticipants();
}

void Manager::removeParticipant(string hostname)
{
    participants_map_mutex.lock();
    participants_map.erase(hostname);
    participants_map_mutex.unlock();
    printParticipants();
}

vector<participant> Manager::getParticipants()
{
    vector<participant> participants;
    participants_map_mutex.lock();
    for (auto const &p : participants_map)
    {
        participants.push_back(p.second);
    }
    this->participants_map_mutex.unlock();
    return participants;
}

void Manager::changeParticipantStatus(string hostname, status s)
{
    participants_map_mutex.lock();
    participants_map[hostname].state = s;
    participants_map_mutex.unlock();
    printParticipants();
}

void Manager::wakeupParticipant(string hostname)
{
    if (debug_mode)
        cout << "wakeupParticipant: waking up " << hostname << endl;
    auto p = this->participants_map[hostname];
    this->sendWakeOnLan(p.mac);
}

void Manager::sendWakeOnLan(string mac)
{
    cout << "sendWakeOnLan: sending wake on lan to mac=" << mac << endl;
    string cmd = "wakeonlan " + mac;
    string res = this->exec(cmd.c_str());
    if (this->debug_mode)
        cout << "sendWakeOnLan: retured " << res << endl;
}