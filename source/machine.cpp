

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <array>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <netdb.h>  // to use hostent
#include <unistd.h> // close

#include "packet.h"
#include "machine.h"

using namespace std;

Machine::Machine(bool is_manager)
{
    this->debug_mode = true;
    this->seqn = 0;
    this->is_manager = is_manager;
    this->hostname = get_hostname();
    this->ip = get_ip();
    this->mac = get_mac();
    this->running = true;
}

void Machine::Start()
{
    thread message_receiver_thread([this]
                                   { message_receiver(PARTICIPANT_PORT); });
    this->interface();
    message_receiver_thread.join();
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
        else
        {
            cout << "Invalid command." << endl;
        }
    };
    this->send_exit();
}

packet *Machine::new_packet(packet_type type)
{
    packet *p = new packet();
    p->type = type;
    p->seqn = this->seqn++;
    p->timestamp = 0; // TODO: get timestamp
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
        char buf[10000];
        unsigned slen = sizeof(sockaddr);
        if (debug_mode)
            cout << "receiveBroadcast: listening for broadcast on port " << from_port << endl;

        int nrecv = recvfrom(sock_fd, buf, sizeof(buf), 0, (sockaddr *)&si_other, &slen);
        if (debug_mode)
            cout << "receiveBroadcast: rcvd packet " << buf << " with len=" << nrecv << endl;

        packet *rcvd_packet = decode_packet(buf);

        this->process_message(rcvd_packet);
    }
}

void Machine::process_message(packet *rcvd_packet)
{
    switch (rcvd_packet->type)
    {
    case DISCOVERY_REQ:
    {
        this->manager_ip = ip;

        if (debug_mode)
            cout << "process_message: received DISCOVERY_REQ packet." << endl;

        int sent_bytes = sendPacket(DISCOVERY_RES, this->manager_ip, MANAGER_PORT, false);
        if (sent_bytes < 0)
            cout << "Error: sendPacket failed." << endl;

        if (debug_mode)
            cout << "process_message: sent DISCOVERY_RES with " << sent_bytes << " bytes"
                 << " to ip:port=" << ip << ":" << MANAGER_PORT << endl;
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
        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &true_int, sizeof(true_int)) < 0)
            printf("ERROR on setsockopt broadcast");
        dst.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else // direct message
    {
        dst.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    packet *p = new_packet(type);
    string encoded_packet = encode_packet(p);
    delete p;

    int len = strlen(encoded_packet.c_str());
    int sent_bytes = sendto(sockfd, encoded_packet.c_str(), len, 0, (const struct sockaddr *)&dst, sizeof(dst));
    ::close(sockfd);
    return sent_bytes;
}

void Machine::send_exit()
{
    if (debug_mode)
        cout << "send_exit: sending EXIT packet to ip:port=" << ip << ":" << MANAGER_PORT << endl;
    sendPacket(EXIT_REQ, this->manager_ip, MANAGER_PORT, false);
}