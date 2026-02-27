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


