
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>

using namespace std;

enum status
{
    awake = 0,
    asleep = 1
};

string StatusToString(status s)
{
    switch (s)
    {
    case awake:
        return "awake";
    case asleep:
        return "asleep";
    default:
        return "unknown";
    }
}

typedef struct __participant
{
    string ip;
    string mac;
    status status;
} participant;
