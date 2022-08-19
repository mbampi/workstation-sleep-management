#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <csignal>

#include "machine.h"
#include "manager.h"
#include "participant.h"

using namespace std;

int main(int argc, char *argv[])
{
    cout << ("Workstation Sleep Manager\n") << endl;

    bool is_manager = false;
    if (argc > 1)
        is_manager = (strcmp(argv[1], "manager") == 0);

    if (is_manager)
    {
        cout << ("Starting as manager\n") << endl;
        Manager *m = new Manager();
        m->Start();
    }
    else
    {
        cout << ("Starting as workstation\n") << endl;
        Participant *p = new Participant();
        p->Start();
    }
}