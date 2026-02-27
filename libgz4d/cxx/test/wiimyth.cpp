#include <gz4d/gz4d.h>
#include <gz4d/WiimoteManager.h>
#include <gz4d/socket/TCPSocket.h>
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    std::string mythaddress = "localhost";
    if(argc > 1)
        mythaddress = argv[1];
    gz4d::TCPSocket mythsocket;
    mythsocket.Connect(gz4d::IPSocket::Address(mythaddress,6546));
    std::string data(1024,0);
    mythsocket.Recv(data);

    gz4d::Wiimote::Ptr wiimote;
    while(true)
    {
        {
            gz4d::WiimoteManager wm;
            gz4d::ThreadedQueue<gz4d::Wiimote::Ptr>::Ptr wq(new gz4d::ThreadedQueue<gz4d::Wiimote::Ptr>);
            wm.AddOutQueue(wq);
            wm.Scan();
            wiimote = wq->pop();
        }
        gz4d::ThreadedQueue<gz4d::Event>::Ptr q(new gz4d::ThreadedQueue<gz4d::Event>);
        wiimote->AddOutQueue(q);
        gz4d::ThreadedQueue<gz4d::Event>::Ptr inq = wiimote->GetInQueue();
        inq->push(gz4d::Event(gz4d::Event::LED,0,9));
        bool done = false;
        gz4d::Time lastEvent = gz4d::time::Now();
        while(!done)
        {
            if(!q->empty())
            {
                gz4d::Event e = q->pop();
                lastEvent = e.timestamp;
                if(e.type == gz4d::Event::Key && e.value)
                {
                    switch (e.id)
                    {
                        case gz4d::Wiimote::A:
                            mythsocket.Send(std::string("key enter\n"));
                            break;
                        case gz4d::Wiimote::B:
                            mythsocket.Send(std::string("key escape\n"));
                            break;
                        case gz4d::Wiimote::Right:
                            mythsocket.Send(std::string("key right\n"));
                            break;
                        case gz4d::Wiimote::Left:
                            mythsocket.Send(std::string("key left\n"));
                            break;
                        case gz4d::Wiimote::Up:
                            mythsocket.Send(std::string("key up\n"));
                            break;
                        case gz4d::Wiimote::Down:
                            mythsocket.Send(std::string("key down\n"));
                            break;
                        case gz4d::Wiimote::One:
                            mythsocket.Send(std::string("key p\n"));
                            break;
                        case gz4d::Wiimote::Two:
                            mythsocket.Send(std::string("key i\n"));
                            break;
                        case gz4d::Wiimote::Home:
                            mythsocket.Send(std::string("jump livetvinguide\n"));
                            break;
                    }
                }
            }
            else
            {
                if(lastEvent + gz4d::time::Duration(30) < gz4d::time::Now())
                {
                    wiimote->close();
                    Sleep(gz4d::seconds(2.5));
                    while(true)
                    {
                        try
                        {
                            wiimote->reopen();
                            inq->push(gz4d::Event(gz4d::Event::LED,0,9));
                            lastEvent = gz4d::time::Now();
                            break;
                        }
                        catch (gz4d::Socket::Exception)
                        {}
                    }
                }
                boost::thread::yield();
            }
            Sleep(gz4d::seconds(0.1));
        }
    }
    return 0;
}
