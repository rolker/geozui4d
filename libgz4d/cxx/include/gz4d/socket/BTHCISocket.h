// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_BTHCISOCKET_H
#define GZ4D_BTHCISOCKET_H

#include <gz4d/socket/BTSocket.h>

#ifndef WIN32
	#include <bluetooth/hci.h>
#endif


namespace gz4d
{
    inline bool operator==(sockaddr_hci const &a, sockaddr_hci const &b)
    {
        return a.hci_dev == b.hci_dev;
    }

    class BTHCISocket: public BTSocket
    {
        public:
            class Address: public BTSocket::Address<sockaddr_hci>
            {
                public:
                    Address(unsigned short device = 0){setDevice(device);}

                    void setDevice(unsigned short device)
                    {
                        _address.hci_dev = htobs(device);
                    }

                    unsigned short getDevice() const
                    {
                        return btohs(_address.hci_dev);
                    }
            };

            BTHCISocket():BTSocket(SOCK_RAW, BTPROTO_HCI){}
    };
};

#endif
