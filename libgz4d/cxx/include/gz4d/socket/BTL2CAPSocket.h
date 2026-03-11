// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_BTL2CAPSOCKET_H
#define GZ4D_BTL2CAPSOCKET_H

#include <gz4d/socket/BTSocket.h>

#ifndef WIN32
    #include <bluetooth/l2cap.h>
#endif

namespace gz4d
{
    inline bool operator== (sockaddr_l2 const &a,sockaddr_l2 const &b)
    {
        return a.l2_bdaddr == b.l2_bdaddr && a.l2_psm == b.l2_psm;
    }

    class BTL2CAPSocket: public BTSocket
    {
        public:

            class Address: public BTSocket::Address<sockaddr_l2>
            {
                public:
                    Address(){}
                    Address(std::string const &address, unsigned short psm=0){setAddress(address); setPSM(psm);}

                    void setAddress(std::string const &addr)
                    {
                        str2ba(addr.c_str(), &_address.l2_bdaddr);
                    }
                    
                    void setPSM(unsigned short psm)
                    {
                        _address.l2_psm = htobs(psm);
                    }
                    
                    std::string getAddress() const
                    {
                        return batostr(&_address.l2_bdaddr);
                    }
                    
                    unsigned short getPSM() const
                    {
                        return btohs(_address.l2_psm);
                    }
            };

            BTL2CAPSocket():BTSocket(SOCK_SEQPACKET, BTPROTO_L2CAP){}
    };

};

#endif
