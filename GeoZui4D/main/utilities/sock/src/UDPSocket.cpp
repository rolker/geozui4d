// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"UDPSocket.h"

UDPSocketNG::UDPSocketNG()
{
    socket = 0;
    in = NULL;
    out = NULL;
}

UDPSocketNG::~ UDPSocketNG()
{
    fclose(in);
    fclose(out);
}


