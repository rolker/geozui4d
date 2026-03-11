// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

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
