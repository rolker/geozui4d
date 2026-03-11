// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOCKET_UDPSENDER_H
#define GZ4D_SOCKET_UDPSENDER_H
#include <gz4d/socket/UDPSocket.h>

namespace gz4d
{
    template <typename T> class UDPSender
    {
        UDPSocket socket;
        public:
            typedef std::shared_ptr<UDPSender> Ptr;

            UDPSender(IPSocket::Address const &a);
            void Consume(T packet);
    };

    template <typename T> inline UDPSender<T>::UDPSender(IPSocket::Address const &a)
    {
        socket.Bind(IPSocket::Address("ANY",0));
        socket.Connect(a);
    }

    template <typename T> inline void UDPSender<T>::Consume(T packet)
    {
        packet->SendVia(socket);
    }

#ifdef GZ4D_BOOST_PYTHON
    template<typename T> void WrapUDPSender(char const *name)
    {
        boost::python::class_<gz4d::UDPSender<T>, std::shared_ptr<UDPSender<T> >, boost::noncopyable >(name, boost::python::init<gz4d::IPSocket::Address const & >())
                ;
    }
#endif

}

#endif
