#ifndef GZ4D_BTRFCOMMSOCKET_H
#define GZ4D_BTRFCOMMSOCKET_H

#include <gz4d/socket/BTSocket.h>

#ifdef WIN32
    #define BTPROTO_RFCOMM BTHPROTO_RFCOMM
	typedef SOCKADDR_BTH sockaddr_rc;
#else
    #include <bluetooth/rfcomm.h>
#endif



namespace gz4d
{
    inline bool operator== (sockaddr_rc const &a,sockaddr_rc const &b)
    {
        return a.rc_bdaddr == b.rc_bdaddr && a.rc_channel == b.rc_channel;
    }

    class BTRFCOMMSocket: public BTSocket
    {
        public:

            class Address: public BTSocket::Address<sockaddr_rc>
            {
                public:
                    Address(){}
                    Address(std::string const &address, uint8_t channel=0){setAddress(address); setChannel(channel);}

                    void setAddress(std::string const &addr)
                    {
                        str2ba(addr.c_str(), &_address.rc_bdaddr);
                    }

                    void setChannel(uint8_t channel)
                    {
                        _address.rc_channel = channel;
                    }

                    std::string getAddress() const
                    {
                        return batostr(&(_address.rc_bdaddr));
                    }

                    uint8_t getChannel() const
                    {
                        return _address.rc_channel;
                    }
            };

            BTRFCOMMSocket():BTSocket(SOCK_STREAM, BTPROTO_RFCOMM){}
    };
};

#endif
