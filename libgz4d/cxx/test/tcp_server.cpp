#include<gz4d/socket/TCPSocket.h>
#include<gz4d/time.h>
#include<iostream>

int main(int argc, char * argv[])
{
    unsigned short p = 2748;
    if (argc == 2)
        p = atoi(argv[1]);
    gz4d::TCPSocket s;
    s.Bind(gz4d::IPSocket::Address("ANY",p));
    s.Listen(1);
    //s.SetNonBlocking();
    while(true)
    {
        std::cout << "Looking for a client...\n";
        gz4d::IPSocket::Address clientAddress;
        gz4d::Socket client = s.Accept(clientAddress);
        int limit = 3;
        std::cout << "Connection from: " << clientAddress.GetAddress() << ":" << clientAddress.GetPort() << "\n";
        client.SetNonBlocking();
        std::string data;
        while(true)
        {
            data.resize(50);
            std::cerr << "Attempt to read data...\n";
            try
            {
                client.Recv(data);
                std::cout << "Received: " << data << "\n";
                gz4d::Sleep(gz4d::seconds(.5));
                std::cout << "Sending response\n";
                client.Send("<"+data+">");
                --limit;
                if(limit <= 0)
                {
                    std::cout << "Enough from this client...\n";
                    break;
                }
            }
            catch(gz4d::Socket::WouldBlockException)
            {
                std::cout << "Sleeping...\n";
                gz4d::Sleep(gz4d::seconds(.1));
            }
            catch(gz4d::Socket::ConnectionClosedException)
            {
                std::cout << "Connection closed by peer\n";
                break;
            }
        }
        std::cout << "Connection closed.\n";
        std::cout << "Doing other stuff....\n";
        gz4d::Sleep(gz4d::seconds(1.5));
    }
    return 0;
}
