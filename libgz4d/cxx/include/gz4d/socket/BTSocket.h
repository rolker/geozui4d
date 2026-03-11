// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_BTSOCKET_H
#define GZ4D_BTSOCKET_H

//gz4d_cmake depends bluetooth

#include <gz4d/socket/Socket.h>

#ifdef WIN32
    #include <ws2bth.h>
    #define AF_BLUETOOTH AF_BTH
    #define PF_BLUETOOTH AF_BTH
#else
    #include <bluetooth/bluetooth.h>
#endif


namespace gz4d
{
    inline bool operator==(bdaddr_t const &a, bdaddr_t const & b)
    {
        for(int i = 0; i < 6; ++i)
            if(a.b[i] != b.b[i])
                return false;
            return true;
    }

    class BTSocket : public Socket
    {
        protected:

            template <typename T> class Address: public Socket::Address<T, AF_BLUETOOTH>
            {
            };

            BTSocket(int type, int protocol):Socket(AF_BLUETOOTH,type,protocol){}
        public:
            BTSocket(){}
    };
};

#endif
