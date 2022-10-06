

#include <string.h>
#include <iostream>
#include <array>
#include <cassert>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <netdb.h>  // to use hostent
#include <unistd.h> // close, sleep
#include <csignal>

#include <iomanip> // to use setw
#include <vector>
#include <mutex>
#include <chrono>

#include "datatypes.h"
#include "machine.h"

using namespace std;

Machine::Machine()
{
    this->debug_mode = true;
    this->seqn = 0;
    this->hostname = getHostname();
    this->ip = getIP();
    this->mac = getMac();
    this->running = true;
    this->manager_ip = "";
    this->is_manager = false;

    this->next_id = 0;
    this->in_election = false;
    this->election_iter = 0;
    this->nro_participants = 0;
}

void Machine::Start()
{
    cout << "start:" << endl;
    thread machine_status_thread([this]
                                 { this->statusRecognizer(); });
    thread messageReceiver_thread([this]
                                  { this->messageReceiver(); });
    thread discovery_thread([this]
                            { this->discovery(); });
    thread monitoring_thread([this]
                             { this->monitoring(); });

    this->interface();
    exit(0);
    discovery_thread.join();
    messageReceiver_thread.join();
    monitoring_thread.join();
    machine_status_thread.join();
}

packet *Machine::newPacket(packet_type type)
{
    packet *p = new packet();
    p->type = type;
    p->seqn = this->seqn++;
    p->sender_ip = this->ip;
    p->sender_hostname = this->hostname;
    p->sender_mac = this->mac;
    if (this->is_manager && type == REPLICATION)
        p->data = this->encodeParticipants();
    else
        p->data = "";
    return p;
}

string Machine::encodeParticipants()
{
    string participants_str = "";
    for (auto &p : this->getParticipants())
    {
        string is_manager_str = p.is_manager ? "1" : "0";
        participants_str += to_string(p.id) + " " + p.hostname + " " + p.ip + " " + p.mac + " " + to_string(p.state) + " " + is_manager_str + "!";
    }
    return participants_str;
}

vector<participant_info> Machine::decodeParticipants(string participants_str)
{
    vector<participant_info> participants;
    string delimiter = "!";
    size_t pos = 0;
    string token;
    while ((pos = participants_str.find(delimiter)) != string::npos)
    {
        token = participants_str.substr(0, pos);
        participant_info p;
        vector<string> part_info = split(token, ' ');
        p.id = stoi(part_info[0]);
        p.hostname = part_info[1];
        p.ip = part_info[2];
        p.mac = part_info[3];
        p.state = status(stoi(part_info[4]));
        p.is_manager = bool(stoi(part_info[5]));
        p.rounds_without_activity = 0;
        participants.push_back(p);
        participants_str.erase(0, pos + delimiter.length());
    }
    return participants;
}

string Machine::getMac()
{
    string macAddr = "";
#if defined(__APPLE__)
    macAddr = exec("ifconfig en1 | awk '/ether/{print $2}'");
    if (macAddr.back() == '\n')
        macAddr.pop_back();
#else // LINUX
    macAddr = exec("cat /sys/class/net/*/address");
    macAddr.erase(macAddr.find('\n'), macAddr.size() - macAddr.find('\n'));
#endif
    return macAddr;
}

string Machine::getHostname()
{
    string hostname = exec("hostname");
    string::size_type pos = hostname.find('.');
    if (pos > 0)
        hostname = hostname.substr(0, pos);
    if (hostname.empty())
        hostname = "unknown hostname";
    if (hostname.back() == '\n')
        hostname.pop_back();

    return hostname;
}

string Machine::getIP()
{
    string ip = exec("ifconfig | grep \"inet \" | grep -Fv 127.0.0.1 | awk '{print $2}'");
    if (ip == "")
        ip = "unknown ip";
    if (ip.back() == '\n')
        ip.pop_back();

    int pos = ip.find(":");
    if (pos != string::npos)
        ip = ip.substr(pos + 1, ip.length());

    return ip;
}
string Machine::getBroadcastIP()
{
    string ip = getIP();
    int pos = ip.find_last_of('.');
    ip = ip.substr(0, pos);
    // ip = ip.append(".63");
    ip = ip.append(".255");
    return ip;
}

string Machine::exec(const char *cmd)
{
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);

    if (!pipe)
        throw runtime_error("popen() failed!");
    while (!feof(pipe.get()))
    {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }

    return result;
}

int Machine::newReceiverSocket()
{
    int from_port = PARTICIPANT_PORT;
    if (this->is_manager)
        from_port = MANAGER_PORT;

    if (this->debug_mode)
        cout << "newReceiverSocket: " << from_port << endl;

    sockaddr_in si_me;
    int sock_fd;
    assert((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1);
    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(from_port);
    si_me.sin_addr.s_addr = INADDR_ANY;
    assert((::bind(sock_fd, (sockaddr *)&si_me, sizeof(sockaddr))) != -1);

    // set timeout
    struct timeval tv;
    tv.tv_sec = WAIT_LIMIT_FOR_MSG;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    return sock_fd;
}

// check when the computer wakes up from sleep,
// setting the machine to manager=false if he was
// the manager before the sleep
// void Machine::statusRecognizer()
// {
//     const int TIMEOUT = 3;
//     auto last_activity_time = chrono::system_clock::now();
//     while (true)
//     {
//         auto current_time = chrono::system_clock::now();
//         chrono::duration<double> elapsed_seconds = current_time - last_activity_time;
//         last_activity_time = current_time;

//         if (elapsed_seconds.count() > TIMEOUT)
//         {
//             // woke up from sleep. We assume he is not the manager anymore
//             this->is_manager = false;
//         }
//         sleep(1);
//     }
// }

void Machine::messageReceiver()
{
    bool current_is_manager = true;
    int msg_not_received = 0;
    int sock_fd = 0;

    while (this->running)
    {
        if (this->is_manager != current_is_manager)
        {
            if (sock_fd != 0)
                close(sock_fd);
            current_is_manager = this->is_manager;
            sock_fd = this->newReceiverSocket();
        }

        int buffer_len = 10000;
        char buf[buffer_len];
        unsigned slen = sizeof(sockaddr);
        if (debug_mode)
        {
            int port = this->is_manager ? MANAGER_PORT : PARTICIPANT_PORT;
            cout << endl
                 << "messageReceiver: listening on port " << port << endl;
        }

        sockaddr_in si_other;
        memset(buf, 0, buffer_len);
        int nrecv = recvfrom(sock_fd, buf, sizeof(buf), 0, (sockaddr *)&si_other, &slen);

        if (debug_mode)
            cout << "messageReceiver: rcvd packet " << buf << " with len=" << nrecv << endl;

        if (nrecv >= 0)
        {
            msg_not_received = 0;
            packet *rcvd_packet = decodePacket(buf);

            if (this->running)
                this->processMessage(rcvd_packet);
        }

        if (msg_not_received >= LIMIT_FOR_ELECTION && !this->in_election && !this->is_manager)
        {
            cout << "messageReceiver: manager not found, begin new election" << endl;
            this->in_election = true;
        }

        election();

        if (this->in_election || election_iter > 0)
            this->election_iter++;
        else
            msg_not_received++;
    }
}

void Machine::sendParticipantsReplicaToAll()
{
    if (this->is_manager)
    {
        if (debug_mode)
            cout << "manager: sendParticipantsReplicaToAll" << endl;

        for (auto &p : this->getParticipants())
        {
            int sent_bytes = this->sendPacket(REPLICATION, p.ip, PARTICIPANT_PORT, false);
            if (debug_mode)
                cout << "manager: sendParticipantsReplicaToAll: ip=" << p.ip << " sent_bytes=" << sent_bytes << endl;
        }
    }
}

int Machine::sendPacket(packet_type type, string to_ip, int to_port, bool broadcast)
{
    int true_int = 1;

    int sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        printf("ERROR opening socket");

    struct sockaddr_in dst;
    dst.sin_family = AF_INET;
    dst.sin_port = htons(to_port);

    if (broadcast)
    {
        string ip = getBroadcastIP();

        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &true_int, sizeof(true_int)) < 0)
            printf("ERROR on setsockopt broadcast");
        // dst.sin_addr.s_addr = htonl(INADDR_ANY);
        dst.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    else // direct message
    {
        dst.sin_addr.s_addr = inet_addr(to_ip.c_str());
    }

    packet *p = newPacket(type);
    string encoded_packet = encodePacket(p);
    delete p;

    int len = strlen(encoded_packet.c_str());
    int sent_bytes = sendto(sockfd, encoded_packet.c_str(), len, 0, (const struct sockaddr *)&dst, sizeof(dst));
    ::close(sockfd);
    return sent_bytes;
}

void Machine::processMessage(packet *rcvd_packet)
{
    if (this->is_manager)
    {
        processMessageAsManager(rcvd_packet);
    }
    else
    {
        processMessageAsParticipant(rcvd_packet);
    }
}

void Machine::processMessageAsParticipant(packet *rcvd_packet)
{
    switch (rcvd_packet->type)
    {
    case DISCOVERY_REQ:
    {
        if (this->manager_ip != rcvd_packet->sender_ip)
        {
            cout << "processMessage: MANAGER hostname=" << rcvd_packet->sender_hostname << " | ip=" << rcvd_packet->sender_ip << " | mac=" << rcvd_packet->sender_mac << endl;
            this->manager_ip = rcvd_packet->sender_ip;
        }

        if (debug_mode)
            cout << "processMessage: received DISCOVERY_REQ packet." << endl;
        int sent_bytes = sendPacket(DISCOVERY_RES, this->manager_ip, MANAGER_PORT, false);
        if (sent_bytes < 0)
            cout << "Error: sendPacket failed." << endl;

        if (debug_mode)
            cout << "processMessage: sent DISCOVERY_RES with " << sent_bytes << " bytes"
                 << " to ip:port=" << this->manager_ip << ":" << MANAGER_PORT << endl;
        break;
    }
    case MONITORING_REQ:
    {
        if (debug_mode)
            cout << "processMessage: received MONITORING_REQ packet." << endl;

        int sent_bytes = sendPacket(MONITORING_RES, this->manager_ip, MANAGER_PORT, false);
        if (sent_bytes < 0)
            printf("processMessage: ERROR sendto");

        if (debug_mode)
            cout << "processMessage: sent MONITORING_RES with " << sent_bytes << " bytes"
                 << " to ip:port=" << this->manager_ip << ":" << MANAGER_PORT << endl;
        break;
    }
    case REPLICATION:
    {
        if (debug_mode)
            cout << "processMessage: received REPLICATION packet." << endl;
        vector<participant_info> participants = decodeParticipants(rcvd_packet->data);
        this->setParticipantsMap(participants);

        this->nro_participants = 0;
        this->next_id = 0;

        for (auto &p : this->getParticipants())
        {
            this->nro_participants++;
            this->next_id = max(p.id, this->next_id);
        }

        this->next_id++;

        break;
    }
    case ELECTION_REQ:
    {
        if (debug_mode)
            cout << "processMessage: received ELECTION_REQ packet." << endl;

        int this_id, sender_id = -1;
        for (auto &p : this->getParticipants())
        {
            if (p.hostname == this->hostname)
                this_id = p.id;

            if (p.hostname == rcvd_packet->sender_hostname)
                sender_id = p.id;
        }

        if (this_id > sender_id)
        {
            int sent_bytes = sendPacket(ELECTION_RES, rcvd_packet->sender_ip, PARTICIPANT_PORT, false);
            this->in_election = true;

            if (sent_bytes < 0)
                printf("processMessage: ERROR sendto");

            if (debug_mode)
                cout << "processMessage: sent ELECTION_RES with " << sent_bytes << " bytes"
                     << " to ip:port=" << rcvd_packet->sender_ip << ":" << PARTICIPANT_PORT << endl;
        }
        else
        {
            this->in_election = false;

            if (election_iter == 0)
                election_iter = 1;

            if (debug_mode)
                cout << "processMessage: This machine has left the election." << endl;
        }
        break;
    }
    case ELECTION_RES:
    {
        if (debug_mode)
            cout << "processMessage: received ELECTION_RES packet." << endl;

        this->in_election = false;

        if (debug_mode)
            cout << "processMessage: This machine has left the election." << endl;

        break;
    }
    case ELECTION_END:
    {
        if (debug_mode)
            cout << "processMessage: received ELECTION_END packet." << endl;

        if (!this->in_election)
        {
            this->election_iter = 0;

            if (debug_mode)
                cout << "processMessage: election ended." << endl;

            cout << "processMessage: MANAGER hostname=" << rcvd_packet->sender_hostname << " | ip=" << rcvd_packet->sender_ip << " | mac=" << rcvd_packet->sender_mac << endl;
            this->manager_ip = rcvd_packet->sender_ip;
        }
        break;
    }
    default:
    {
        if (debug_mode)
            cout << "processMessage: Received UNKNOWN packet" << endl;
        break;
    }
    }
}

void Machine::setParticipantsMap(vector<participant_info> new_participants)
{
    this->participantsMapMutex.lock();
    this->participantsMap.clear();
    for (auto p : new_participants)
    {
        this->participantsMap[p.hostname] = p;
    }
    this->participantsMapMutex.unlock();
    printParticipants();
}

void Machine::processMessageAsManager(packet *rcvd_packet)
{
    if (debug_mode)
        cout << "manager: processMessage: received packet" << endl;
    switch (rcvd_packet->type)
    {
    case DISCOVERY_RES:
    {
        if (debug_mode)
            cout << "processMessage: Received DISCOVERY_RES" << endl;

        participant_info *p = new participant_info();
        p->id = this->next_id;
        p->ip = rcvd_packet->sender_ip;
        p->mac = rcvd_packet->sender_mac;
        p->hostname = rcvd_packet->sender_hostname;
        p->state = awake;
        p->is_manager = false;
        p->rounds_without_activity = 0;
        this->addParticipant(p);

        if (debug_mode)
            cout << "processMessageAsManager: NEXT ID = " << this->next_id << endl;

        break;
    }
    case MONITORING_RES:
    {
        if (debug_mode)
            cout << "processMessage: Received MONITORING_RES in port " << endl;
        zeroRoundsWithoutActivity(rcvd_packet->sender_hostname);
        changeParticipantStatus(rcvd_packet->sender_hostname, awake);
        break;
    }
    case EXIT_REQ:
    {
        if (debug_mode)
            cout << "processMessage: Received EXIT_REQ" << endl;
        removeParticipant(rcvd_packet->sender_hostname);
        break;
    }
    default:
    {
        if (debug_mode)
            cout << "processMessage: Received UNKNOWN packet" << endl;
        break;
    }
    }
}

void Machine::interface()
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
        else if ((cmd == "WAKEUP") && (this->is_manager))
        {
            string hostname = input.substr(input.find(" ") + 1);
            this->wakeupParticipant(hostname);
        }
        else
        {
            cout << "Invalid command." << endl;
        }
    };
    this->running = false;

    if (!this->is_manager)
        this->sendExit();
}

void Machine::sendExit()
{
    if (!(this->is_manager))
    {
        if (debug_mode)
            cout << "sendExit: sending EXIT packet to ip:port=" << manager_ip << ":" << MANAGER_PORT << endl;
        sendPacket(EXIT_REQ, this->manager_ip, MANAGER_PORT, false);
    }
}

void Machine::monitoring()
{
    do
    {
        if (this->is_manager)
        {
            if (debug_mode)
                cout << "manager: monitoring: sending req" << endl;

            for (auto &p : this->getParticipants())
            {
                if (p.hostname != this->hostname)
                {
                    if (p.rounds_without_activity >= ROUNDS_WITHOUT_ACTIVITY_THRESHOLD)
                        this->changeParticipantStatus(p.hostname, asleep);
                    this->incRoundsWithoutActivity(p.hostname);
                    int sent_bytes = this->sendPacket(MONITORING_REQ, p.ip, PARTICIPANT_PORT, false);
                    if (debug_mode)
                        cout << "manager: monitoring: sent_bytes=" << sent_bytes << endl;
                }
            }
        }

        sleep(MONITORING_INTERVAL);
    } while (this->running);
}

void Machine::discovery()
{
    do
    {
        if (this->is_manager)
        {
            if (debug_mode)
                cout << "manager: discovery: sending req" << endl;
            this->sendPacket(DISCOVERY_REQ, "", PARTICIPANT_PORT, true);
        }
        sleep(DISCOVERY_INTERVAL);
    } while (this->running);
}

void Machine::statusRecognizer()
{
    int from_port = PARTICIPANT_PORT;

    sockaddr_in si_me;
    int sock_fd;
    assert((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1);
    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(from_port);
    si_me.sin_addr.s_addr = INADDR_ANY;
    assert((::bind(sock_fd, (sockaddr *)&si_me, sizeof(sockaddr))) != -1);

    while (this->running)
    {
        if (this->is_manager)
        {

            int buffer_len = 10000;
            char buf[buffer_len];
            unsigned slen = sizeof(sockaddr);
            if (debug_mode)
            {
                cout << endl
                     << "statusRecognizer: listening on port " << PARTICIPANT_PORT << endl;
            }

            sockaddr_in si_other;
            memset(buf, 0, buffer_len);
            int nrecv = recvfrom(sock_fd, buf, sizeof(buf), 0, (sockaddr *)&si_other, &slen);

            if (debug_mode)
                cout << "statusRecognizer: rcvd packet " << buf << " with len=" << nrecv << endl;

            if (nrecv >= 0)
            {
                packet *rcvd_packet = decodePacket(buf);

                if (this->running)
                {
                    if (rcvd_packet->sender_ip != this->ip)
                    {
                        this->is_manager = false;
                        this->in_election = true;
                    }
                }
            }
        }
    }
}

void Machine::printParticipants()
{
    cout << setw(15) << "\nPARTICIPANTS" << endl;
    cout << left << setw(10)
         << "ID"
         << left << setw(25)
         << "Hostname"
         << left << setw(25)
         << "IP"
         << left << setw(25)
         << "MAC"
         << left << setw(10)
         << "Status"
         << left << setw(10)
         << "Manager"
         << endl;
    for (const participant_info p : getParticipants())
    {
        cout << left << setw(10)
             << p.id
             << left << setw(25)
             << p.hostname
             << left << setw(25)
             << p.ip
             << left << setw(25)
             << p.mac
             << left << setw(10)
             << status_to_string(p.state)
             << left << setw(10)
             << (p.is_manager ? "Yes" : "No")
             << endl;
    }
}

void Machine::addParticipant(participant_info *p)
{
    if (debug_mode)
        cout << "\naddParticipant: adding participant " << p->hostname << endl;
    // check if participant already exists
    for (auto &participant : this->getParticipants())
    {
        if (participant.hostname == p->hostname)
        {
            if (debug_mode)
                cout << "addParticipant: participant already exists" << endl;
            return;
        }
    }
    participantsMapMutex.lock();
    participantsMap[p->hostname] = *p;
    participantsMapMutex.unlock();

    this->nro_participants++;
    this->next_id++;
    printParticipants();
    sendParticipantsReplicaToAll();
}

void Machine::removeParticipant(string hostname)
{
    participantsMapMutex.lock();
    participantsMap.erase(hostname);
    participantsMapMutex.unlock();

    this->nro_participants--;
    printParticipants();
    sendParticipantsReplicaToAll();
}

vector<participant_info> Machine::getParticipants()
{
    vector<participant_info> participants;
    participantsMapMutex.lock();
    for (auto const &p : participantsMap)
    {
        participants.push_back(p.second);
    }
    this->participantsMapMutex.unlock();
    return participants;
}

void Machine::changeParticipantStatus(string hostname, status s)
{
    status oldStatus = participantsMap[hostname].state;
    if (oldStatus == s)
        return;

    participantsMapMutex.lock();
    participantsMap[hostname].state = s;
    if (participantsMap[hostname].is_manager && s == asleep)
        participantsMap[hostname].is_manager = false;
    participantsMapMutex.unlock();

    printParticipants();
    sendParticipantsReplicaToAll();
}

void Machine::incRoundsWithoutActivity(string hostname)
{
    participantsMapMutex.lock();
    participantsMap[hostname].rounds_without_activity++;
    participantsMapMutex.unlock();
}

void Machine::zeroRoundsWithoutActivity(string hostname)
{
    participantsMapMutex.lock();
    participantsMap[hostname].rounds_without_activity = 0;
    participantsMapMutex.unlock();
}

void Machine::wakeupParticipant(string hostname)
{
    if (debug_mode)
        cout << "wakeupParticipant: waking up " << hostname << endl;
    auto p = this->participantsMap[hostname];
    this->sendWakeOnLan(p.mac);
}

void Machine::sendWakeOnLan(string mac)
{
    cout << "sendWakeOnLan: sending wake on lan to mac=" << mac << endl;
    string cmd = "wakeonlan " + mac;
    string res = this->exec(cmd.c_str());
    if (this->debug_mode)
        cout << "sendWakeOnLan: retured " << res << endl;
}

void Machine::setSelfAsManager()
{
    this->is_manager = true;
    this->manager_ip = this->ip;

    participant_info *p = new participant_info();
    p->hostname = this->hostname;
    p->ip = this->ip;
    p->mac = this->mac;
    p->state = awake;
    p->rounds_without_activity = 0;
    p->is_manager = true;

    p->id = this->next_id;
    this->next_id++;

    if (participantsMap[p->hostname].hostname == this->hostname)
    {
        p->id = participantsMap[p->hostname].id;
        this->next_id = 0;
        for (auto &part : this->getParticipants())
        {
            this->next_id = max(part.id, this->next_id);
        }

        this->nro_participants++;
    }
    participantsMapMutex.lock();
    participantsMap[p->hostname] = *p;
    participantsMapMutex.unlock();

    printParticipants();
    sendParticipantsReplicaToAll();
}

void Machine::election()
{
    int T = 3 * this->nro_participants;

    if (this->nro_participants > 1)
    {
        if (this->in_election)
        {
            if (this->election_iter == 0)
            {
                // Se existem outros participantes, este ainda está concorrendo e NÃO HOUVE iterações de eleição ainda
                // Manda eleição para lista de participantes
                for (auto &p : this->getParticipants())
                {
                    if (p.hostname != this->hostname)
                    {
                        int sent_bytes = this->sendPacket(ELECTION_REQ, p.ip, PARTICIPANT_PORT, false);
                        if (debug_mode)
                            cout << "election: Begin: sent_bytes=" << sent_bytes << endl;
                    }
                }
            }
            else if (this->election_iter >= LIMIT_FOR_ELECTION)
            {
                // Se existem outros participantes, este ainda está concorrendo, NÃO HOUVE resposta e ACABOU
                // Avisa todos os participantes que acabou e este é o novo manager, zera as variáveis de eleição, se seta como manager

                for (auto &p : this->getParticipants())
                {
                    if (p.hostname != this->hostname)
                    {
                        int sent_bytes = this->sendPacket(ELECTION_END, p.ip, PARTICIPANT_PORT, false);
                        if (debug_mode)
                            cout << "election: End: sent_bytes=" << sent_bytes << endl;
                    }
                }
                this->in_election = false;
                this->election_iter = 0;

                this->setSelfAsManager();

                if (this->debug_mode)
                    cout << "election: I am the new manager!" << endl;
            }
        }
        else
        {
            if (this->election_iter >= T)
            {
                // Se existem outros participantes, este NÃO está concorrendo, HOUVE iterações de eleição e ACABOU o tempo
                // Reseta variáveis para começar uma nova eleição
                this->in_election = true;
                this->election_iter = -1;

                cout << "election: End: new manager failed, begin new election" << endl;
            }
        }
    }
    else
    {
        if (this->in_election)
        {
            // Se não existem outros participantes
            // zera as variáveis de eleição, se seta como manager
            this->in_election = false;
            this->election_iter = 0;
            this->setSelfAsManager();

            if (this->debug_mode)
                cout << "election: I am the new manager!" << endl;
        }
    }
}
