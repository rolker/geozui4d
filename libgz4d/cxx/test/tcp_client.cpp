// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gz4d/socket/TCPSocket.h>
#include <gz4d/time.h>
#include<iostream>

int main(int argc, char * argv[])
{
    unsigned short p = 2748;
    if (argc == 2)
        p = atoi(argv[1]);
    gz4d::TCPSocket s;
    s.Connect(gz4d::IPSocket::Address("LOOPBACK",p));
    gz4d::IPSocket::Address a;
    s.GetRemoteAddress(a);
    std::cout << "Connected to: " << a.GetAddress() << ":" << a.GetPort() << "\n";
    s.SetNonBlocking();
    for (int i=0; i<5; i++)
    {
        try
        {
            s.Send(std::string("Hello!"));
        }
        catch(gz4d::Socket::Exception e)
        {
            std::cerr << "send failed: " << e.what() << std::endl;
            break;
        }
        std::cerr << "Sent.\n";
        std::string answer;
        answer.resize(25);
        while(true)
            try
            {
                s.Recv(answer);
                std::cout << answer << "\n";
                break;
            }
            catch(gz4d::Socket::WouldBlockException)
            {
                std::cerr << "Sleep.\n";
                gz4d::Sleep(gz4d::seconds(.1));
            }
            catch(gz4d::Socket::Exception e)
            {
                std::cerr << "recv failed: " << e.what() << std::endl;
                i=5;
                break;
            }
        gz4d::Sleep(gz4d::seconds(.5));
    }
    return 0;
}
