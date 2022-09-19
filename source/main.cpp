#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <csignal>

#include "machine.h"

using namespace std;

int main(int argc, char *argv[])
{
    cout << ("Workstation Sleep Manager\n") << endl;

    bool is_manager = false;
    if (argc > 1)
        is_manager = (strcmp(argv[1], "manager") == 0);

    Machine *m = new Machine(is_manager);
    m->Start();
}