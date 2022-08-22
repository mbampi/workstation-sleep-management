
#include "manager.h"

void Manager::Start()
{
    thread message_receiver_thread([this]
                                   { this->message_receiver(MANAGER_PORT); });
    thread discovery_thread([this]
                            { this->discovery(); });
    thread monitoring_thread([this]
                             { this->monitoring(); });

    this->interface();

    discovery_thread.join();
    message_receiver_thread.join();
    monitoring_thread.join();
}

void Manager::interface()
{
    string input;
    while (getline(cin, input))
    {
        string cmd = input.substr(0, input.find(" "));
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if (cmd == "EXIT")
        {
            cout << "Exiting..." << endl;
            this->running = false;
            break;
        }
        else if (cmd == "DEBUG")
        {
            this->debug_mode = !this->debug_mode;
            cout << "Debug mode: " << (this->debug_mode ? "on" : "off") << endl;
        }
        else if (cmd == "PRINT")
        {
            this->printParticipants();
        }
        else if (cmd == "WAKEUP")
        {
            string hostname = input.substr(input.find(" ") + 1);
            this->wakeupParticipant(hostname);
        }
        else
        {
            cout << "Invalid command." << endl;
        }
    };
}

void Manager::monitoring()
{
    do
    {
        if (debug_mode)
            cout << "manager: monitoring: sending req" << endl;

        for (auto &p : this->getParticipants())
        {
            if (p.rounds_without_activity >= ROUNDS_WITHOUT_ACTIVITY_THRESHOLD)
                this->changeParticipantStatus(p.hostname, asleep);
            this->inc_rounds_without_activity(p.hostname);
            int sent_bytes = this->sendPacket(MONITORING_REQ, p.ip, PARTICIPANT_PORT, false);
            if (debug_mode)
                cout << "manager: monitoring: sent_bytes=" << sent_bytes << endl;
        }

        sleep(MONITORING_INTERVAL);
    } while (this->running);
}

void Manager::discovery()
{
    do
    {
        if (debug_mode)
            cout << "manager: discovery: sending req" << endl;
        this->sendPacket(DISCOVERY_REQ, "", PARTICIPANT_PORT, true);

        sleep(DISCOVERY_INTERVAL);
    } while (this->running);
}

void Manager::process_message(packet *rcvd_packet)
{
    if (debug_mode)
        cout << "manager: process_message: received packet" << endl;
    switch (rcvd_packet->type)
    {
    case DISCOVERY_RES:
    {
        if (debug_mode)
            cout << "process_message: Received DISCOVERY_RES" << endl;

        participant_info *p = new participant_info();
        p->ip = rcvd_packet->sender_ip;
        p->mac = rcvd_packet->sender_mac;
        p->hostname = rcvd_packet->sender_hostname;
        p->state = awake;
        p->rounds_without_activity = 0;
        this->addParticipant(p);

        break;
    }
    case MONITORING_RES:
    {
        if (debug_mode)
            cout << "process_message: Received MONITORING_RES" << endl;
        zero_rounds_without_activity(rcvd_packet->sender_hostname);
        changeParticipantStatus(rcvd_packet->sender_hostname, awake);
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
    for (const participant_info p : getParticipants())
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

void Manager::addParticipant(participant_info *p)
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

vector<participant_info> Manager::getParticipants()
{
    vector<participant_info> participants;
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

void Manager::inc_rounds_without_activity(string hostname)
{
    participants_map_mutex.lock();
    participants_map[hostname].rounds_without_activity++;
    participants_map_mutex.unlock();
}

void Manager::zero_rounds_without_activity(string hostname)
{
    participants_map_mutex.lock();
    participants_map[hostname].rounds_without_activity = 0;
    participants_map_mutex.unlock();
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
