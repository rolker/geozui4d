#ifndef UDPSOCKETNG_H
#define UDPSOCKETNG_H

#include "stdio.h"

class UDPSocketNG
{
public:
    UDPSocketNG();
    ~UDPSocketNG();
private:
    int socket;
    FILE * in;
    FILE * out;
};


#endif
