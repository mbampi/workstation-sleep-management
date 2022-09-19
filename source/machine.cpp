

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

#include "datatypes.h"
#include "machine.h"

using namespace std;

Machine::Machine(bool is_manager)
{
    this->debug_mode = true;
    this->seqn = 0;
    this->hostname = get_hostname();
    this->ip = get_ip();
    this->mac = get_mac();
    this->running = true;
    this->manager_ip = "";
    this->is_manager = is_manager;
}

void Machine::Start()
{
    cout << "start:" << endl;
    if (this->is_manager)
    {
        manager_start();
    }
    else
    {
        participant_start();
    }
}

void Machine::participant_start()
{
    thread message_receiver_thread([this]
                                   { message_receiver(PARTICIPANT_PORT); });
    this->interface();
    exit(0);
    message_receiver_thread.join();
}

void Machine::manager_start()
{
    thread message_receiver_thread([this]
                                   { this->message_receiver(MANAGER_PORT); });
    thread discovery_thread([this]
                            { this->discovery(); });
    thread monitoring_thread([this]
                             { this->monitoring(); });

    this->interface();
    exit(0);
    discovery_thread.join();
    message_receiver_thread.join();
    monitoring_thread.join();
}

packet *Machine::new_packet(packet_type type)
{
    packet *p = new packet();
    p->type = type;
    p->seqn = this->seqn++;
    p->sender_ip = this->ip;
    p->sender_hostname = this->hostname;
    p->sender_mac = this->mac;
    return p;
}

string Machine::get_mac()
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

string Machine::get_hostname()
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

string Machine::get_ip()
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
string Machine::get_broadcast_ip()
{
    string ip = get_ip();
    // int pos = ip.find_last_of('.');
    // ip = ip.substr(0, pos);
    // ip = ip.append(".63");
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

void Machine::message_receiver(int from_port)
{
    sockaddr_in si_me, si_other;
    int sock_fd;
    assert((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1);

    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(from_port);
    si_me.sin_addr.s_addr = INADDR_ANY;

    assert((::bind(sock_fd, (sockaddr *)&si_me, sizeof(sockaddr))) != -1);

    while (this->running)
    {
        if (debug_mode)
            cout << endl;
        int buffer_len = 10000;
        char buf[buffer_len];
        unsigned slen = sizeof(sockaddr);
        if (debug_mode)
            cout << "message_receiver: listening on port " << from_port << endl;

        memset(buf, 0, buffer_len);
        int nrecv = recvfrom(sock_fd, buf, sizeof(buf), 0, (sockaddr *)&si_other, &slen);

        if (debug_mode)
            cout << "message_receiver: rcvd packet " << buf << " with len=" << nrecv << endl;

        packet *rcvd_packet = decode_packet(buf);

        if (this->running)
            this->process_message(rcvd_packet);
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
        string ip = get_broadcast_ip();

        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &true_int, sizeof(true_int)) < 0)
            printf("ERROR on setsockopt broadcast");
        // dst.sin_addr.s_addr = htonl(INADDR_ANY);
        dst.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    else // direct message
    {
        dst.sin_addr.s_addr = inet_addr(to_ip.c_str());
    }

    packet *p = new_packet(type);
    string encoded_packet = encode_packet(p);
    delete p;

    int len = strlen(encoded_packet.c_str());
    int sent_bytes = sendto(sockfd, encoded_packet.c_str(), len, 0, (const struct sockaddr *)&dst, sizeof(dst));
    ::close(sockfd);
    return sent_bytes;
}

void Machine::process_message(packet *rcvd_packet)
{
    if (this->is_manager)
    {
        process_message_as_manager(rcvd_packet);
    }
    else
    {
        process_message_as_participant(rcvd_packet);
    }
}

void Machine::process_message_as_participant(packet *rcvd_packet)
{
    if (this->manager_ip == "")
        cout << "MANAGER hostname=" << rcvd_packet->sender_hostname << " | ip=" << rcvd_packet->sender_ip << " | mac=" << rcvd_packet->sender_mac << endl;
    this->manager_ip = rcvd_packet->sender_ip;
    switch (rcvd_packet->type)
    {
    case DISCOVERY_REQ:
    {
        if (debug_mode)
            cout << "process_message: received DISCOVERY_REQ packet." << endl;
        int sent_bytes = sendPacket(DISCOVERY_RES, this->manager_ip, MANAGER_PORT, false);
        if (sent_bytes < 0)
            cout << "Error: sendPacket failed." << endl;

        if (debug_mode)
            cout << "process_message: sent DISCOVERY_RES with " << sent_bytes << " bytes"
                 << " to ip:port=" << manager_ip << ":" << MANAGER_PORT << endl;
        break;
    }
    case MONITORING_REQ:
    {
        if (debug_mode)
            cout << "process_message: received MONITORING_REQ packet." << endl;

        int sent_bytes = sendPacket(MONITORING_RES, this->manager_ip, MANAGER_PORT, false);
        if (sent_bytes < 0)
            printf("ERROR sendto");

        if (debug_mode)
            cout << "process_message: sent MONITORING_RES with " << sent_bytes << " bytes"
                 << " to ip:port=" << ip << ":" << MANAGER_PORT << endl;
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

void Machine::process_message_as_manager(packet *rcvd_packet)
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

void Machine::interface()
{
    if (this->is_manager)
    {
        manager_interface();
    }
    else
    {
        participant_interface();
    }
}

void Machine::participant_interface()
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
        else
        {
            cout << "Invalid command." << endl;
        }
    };
    this->running = false;
    this->send_exit();
}

void Machine::manager_interface()
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
    }
    this->running = false;
}

void Machine::send_exit()
{
    if (this->is_manager)
    {
    }
    else
    {
        participant_send_exit();
    }
}

void Machine::participant_send_exit()
{
    if (debug_mode)
        cout << "send_exit: sending EXIT packet to ip:port=" << manager_ip << ":" << MANAGER_PORT << endl;
    sendPacket(EXIT_REQ, this->manager_ip, MANAGER_PORT, false);
}

void Machine::monitoring()
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

void Machine::discovery()
{
    do
    {
        if (debug_mode)
            cout << "manager: discovery: sending req" << endl;
        this->sendPacket(DISCOVERY_REQ, "", PARTICIPANT_PORT, true);

        sleep(DISCOVERY_INTERVAL);
    } while (this->running);
}

void Machine::printParticipants()
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

void Machine::addParticipant(participant_info *p)
{
    if (debug_mode)
        cout << "\naddParticipant: adding participant " << p->hostname << endl;
    participants_map_mutex.lock();
    participants_map[p->hostname] = *p;
    participants_map_mutex.unlock();
    printParticipants();
}

void Machine::removeParticipant(string hostname)
{
    participants_map_mutex.lock();
    participants_map.erase(hostname);
    participants_map_mutex.unlock();
    printParticipants();
}

vector<participant_info> Machine::getParticipants()
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

void Machine::changeParticipantStatus(string hostname, status s)
{
    participants_map_mutex.lock();
    participants_map[hostname].state = s;
    participants_map_mutex.unlock();
    printParticipants();
}

void Machine::inc_rounds_without_activity(string hostname)
{
    participants_map_mutex.lock();
    participants_map[hostname].rounds_without_activity++;
    participants_map_mutex.unlock();
}

void Machine::zero_rounds_without_activity(string hostname)
{
    participants_map_mutex.lock();
    participants_map[hostname].rounds_without_activity = 0;
    participants_map_mutex.unlock();
}

void Machine::wakeupParticipant(string hostname)
{
    if (debug_mode)
        cout << "wakeupParticipant: waking up " << hostname << endl;
    auto p = this->participants_map[hostname];
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
