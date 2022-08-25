
#include "participant.h"

void Participant::Start()
{
    thread message_receiver_thread([this]
                                   { message_receiver(PARTICIPANT_PORT); });
    this->interface();
    exit(0);
    message_receiver_thread.join();
}

void Participant::interface()
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

void Participant::process_message(packet *rcvd_packet)
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

void Participant::send_exit()
{
    if (debug_mode)
        cout << "send_exit: sending EXIT packet to ip:port=" << manager_ip << ":" << MANAGER_PORT << endl;
    sendPacket(EXIT_REQ, this->manager_ip, MANAGER_PORT, false);
}
