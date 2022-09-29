#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <csignal>

#include "machine.h"

using namespace std;

int main(int argc, char *argv[])
{
    cout << ("Workstation Sleep Manager\n") << endl;

    Machine *m = new Machine();
    m->Start();
}