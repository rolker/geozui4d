// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/socket/IPSocket.h>
#include <gz4d/socket/UDPSocket.h>
#include <iostream>
#include <string>

void PrintAddress(gz4d::Socket &s, std::string label)
{
    gz4d::IPSocket::Address la,ra;

    try
    {
        s.GetLocalAddress(la);
        std::cout << label << " local:\t" << la.GetPort() << "\t" << la.GetAddress() << "\n";
        s.GetRemoteAddress(ra);
        std::cout << label << " remote:\t" << ra.GetPort() << "\t" << ra.GetAddress() << "\n";
    }
    catch(gz4d::Socket::Exception e)
    {
        std::cerr << "Caught exception in PrintAddress: " << e.what() << std::endl;
    }

}

int main(int argc, char * argv[])
{
    gz4d::UDPSocket s;
    gz4d::IPSocket::Address la,ra;

    gz4d::Socket bad = gz4d::Socket();

    PrintAddress(bad,"Bad socket");

    bad = gz4d::Socket(123);

    PrintAddress(bad,"Bad socket");

    PrintAddress(s,"Pre bind");

    s.Bind(gz4d::IPSocket::Address("ANY",0));

    PrintAddress(s,"Post bind");

    bad = gz4d::Socket(s);
    PrintAddress(bad,"Copied");


    gz4d::UDPSocket s2;

    s.GetLocalAddress(la);

    s2.Bind(gz4d::IPSocket::Address("ANY",0));

    s2.Connect(gz4d::IPSocket::Address("LOOPBACK",la.GetPort()));

    PrintAddress(s2,"s2 loopback");

    s2.Send(std::string("Hello!"));
    std::string data;
    data.resize(100);
    s.RecvFrom(data,ra);

    std::cout << data.size() << " bytes from " << ra.GetAddress() << ":" << ra.GetPort() << "\n";
    std::cout << data << "\n";
    return 0;
}
