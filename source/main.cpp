#include <iostream>
#include "manager.cpp"
#include "participant.cpp"

typedef struct __packet
{
    uint16_t type;        // Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;        // Número de sequência
    uint16_t length;      // Comprimento do payload
    uint16_t timestamp;   // Timestamp do dado
    const char *_payload; // Dados da mensagem
} packet;

int main(int argc, char *argv[])
{
    std::cout << ("Workstation Sleep Manager\n") << std::endl;

    bool isManager = false;
    if (argc > 1)
        isManager = (strcmp(argv[1], "manager") == 0);

    std::cout << ("Started") << std::endl;

    if (isManager)
    {
        std::cout << ("Manager mode") << std::endl;
        startManager();
        return 0;
    }
    else
    {
        std::cout << ("Participant mode") << std::endl;
        startParticipant();
        return 0;
    }
}