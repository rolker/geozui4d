#ifndef GZ4D_SIMRAD_H
#define GZ4D_SIMRAD_H

#include <gz4d/gz4d.h>
#include <gz4d/socket/TCPSocket.h>
#include <iostream>
#include <cstring>


namespace gz4d
{
    class DeltaT
    {
#pragma pack(1)
        class PingRequest
        {
            unsigned char head1;
            unsigned char head2;
            unsigned char id;
            unsigned char range;
            uint32_t reserved1;
            unsigned char startGain;
            unsigned char reserved2; // 1
            unsigned char reserved3; // 10
            uint16_t reserved4;
            unsigned char packetNumber;
            unsigned char pulseLength;
            unsigned char reserved5;
            unsigned char externalTrigger;
            uint16_t triggerDelay;
            unsigned char dataPoints;
            unsigned char dataBits;
            uint16_t reserved6;
            unsigned char reserved7;
            unsigned char delay;
            unsigned char frequency;
            unsigned char term;
            public:
                PingRequest()
                :head1(0xFE),head2(0x44),id(0x10),reserved1(0),reserved2(1),reserved3(10),reserved4(0),reserved5(0),packetNumber(0),externalTrigger(0),triggerDelay(0),dataPoints(8),dataBits(8),reserved6(0),reserved7(0),delay(0),frequency(86),term(0xFD)
                {
                }
                
                void setRange(unsigned int m)
                {
                    range = m;
                }
                
                void setStartGain(unsigned int db)
                {
                    startGain = db;
                }
                
                void setPulseLength(unsigned int us)
                {
                    pulseLength = us/10;
                }
                
                void inc(){++packetNumber;}
        };
        
        struct PingResponse
        {
            unsigned char head[3];
            unsigned char id;
            unsigned char serialStatus;
            unsigned char packetNumber;
            unsigned char firmware;
            unsigned char range;
            uint16_t reserved1;
            uint16_t dataBytes;
            unsigned char triggerStatus;
            unsigned char reserved2[19];
            unsigned char data;
            unsigned char term;
        };
#pragma pack()

        unsigned int range;
        unsigned int gain;
        unsigned int pulseLength;
        TCPSocket head;
        public:
            DeltaT()
            :range(5),gain(20),pulseLength(30)
            {
            }
            
            void Connect(IPSocket::Address const &address)
            {
                head.Bind(IPSocket::Address());
                head.Connect(address);
            }
            
            std::vector<unsigned char> Ping()
            {
                std::vector<unsigned char> ret(8000);
                PingRequest pr;
                pr.setRange(range);
                pr.setStartGain(gain);
                pr.setPulseLength(pulseLength);
                for(unsigned int i = 0; i < 8; ++i)
                {
                    head.Send(reinterpret_cast<const char*>(&pr),sizeof(pr));
                    Socket::Packet ret_packet;
                    ret_packet.resize(1033);
                    head.Recv(ret_packet);
                    PingResponse *response = reinterpret_cast<PingResponse *>(ret_packet.front());
                    memcpy(&(ret[i*1000]),&(response->data),1000);
                    pr.inc();
                }
                return ret;
            }
            
            
    };
}

#endif
