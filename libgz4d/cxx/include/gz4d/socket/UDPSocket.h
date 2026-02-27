#ifndef GZ4D_UDPSOCKET_H
#define GZ4D_UDPSOCKET_H

#include <gz4d/socket/IPSocket.h>

namespace gz4d
{
    class UDPSocket: public IPSocket
    {
        public:
            UDPSocket():IPSocket(SOCK_DGRAM){};
    };
};

#endif
