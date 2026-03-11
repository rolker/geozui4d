// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOCKETPRODUCER_H
#define GZ4D_SOCKETPRODUCER_H

#include <gz4d/gz4d.h>
#include <gz4d/socket/Socket.h>
#include <gz4d/thread/Producer.h>

namespace gz4d
{
    class SocketProducer: public Socket, public Producer<std::shared_ptr<Socket::Packet> >
    {
        public:
            SocketProducer(){}
            SocketProducer(Socket const &s):Socket(s){}
            SocketProducer(SocketProducer const &s):Socket(s),bufferSize(s.bufferSize){}
            inline void Main()
            {
                std::shared_ptr<gz4d::Socket::Packet> data;
                while(true)
                {
                    std::shared_ptr<gz4d::Socket::Packet> data(new gz4d::Socket::Packet);
                    data->resize(bufferSize);
                    Recv(*data);
                    Producer<std::shared_ptr<Socket::Packet> >::Send(data);
                }
            }
            void SetBufferSize(unsigned int bs){bufferSize = bs;}
            unsigned int GetBufferSize(){return bufferSize;}
        private:
            unsigned int bufferSize;
    };
}

#endif
