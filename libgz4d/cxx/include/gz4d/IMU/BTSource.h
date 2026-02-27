#ifndef GZ4D_IMU_BTSOURCE_H
#define GZ4D_IMU_BTSOURCE_H

#include <gz4d/IMU.h>
#include <gz4d/socket/BTRFCOMMSocket.h>
#include <boost/thread/thread.hpp>
#include <gz4d/thread/Producer.h>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>


namespace gz4d
{
    namespace IMU
    {
    
        /**
        \ingroup devices
        Bluetooth Interface to SparkFun's 6DOF Bluetooth IMU.
        */
        class BTSource: public Producer<RawState>
        {
            BTRFCOMMSocket socket;
            boost::thread socketThread;
            void SocketThread()
            {
                std::vector<uint8_t> buffer(22,0);
                std::vector<uint8_t>::iterator cursor = buffer.end();
                std::vector<uint8_t> packet;
                
                while (!boost::this_thread::interruption_requested())
                {
                    packet.resize(256);
                    try
                    {
                        socket.Recv(packet);
                        if ( !packet.empty() )
                        {
                            std::vector<uint8_t>::iterator pcursor = packet.begin();
                            while (pcursor != packet.end())
                            {
                                if (cursor == buffer.end()) // is our buffer empty?
                    {
                        cursor = buffer.begin();
                        // find next start of packet
                        while (*pcursor != 'A' && pcursor != packet.end())
                            ++pcursor;
                    }
                    while (cursor != buffer.end() && pcursor != packet.end())
                    {
                        *cursor = *pcursor;
                        ++cursor;
                        ++pcursor;
                    }
                    if (cursor == buffer.end()) // full buffer
                    {
                        if (buffer[0] == 'A' && buffer[21] == 'Z')
                        {
                            RawState r;
                            r.time = gz4d::time::Now();
                            r.count = (buffer[1] << 8 | buffer[2]);
                            r.mag[0] = (buffer[3] << 8 | buffer[4]);
                            r.mag[1] = (buffer[5] << 8 | buffer[6]);
                            r.mag[2] = (buffer[7] << 8 | buffer[8]);
                            r.accel[0] = (buffer[9] << 8 | buffer[10]);
                            r.accel[1] = (buffer[11] << 8 | buffer[12]);
                            r.accel[2] = (buffer[13] << 8 | buffer[14]);
                            r.gyro[0]  = (buffer[15] << 8 | buffer[16]);
                            r.gyro[1] = (buffer[17] << 8 | buffer[18]);
                            r.gyro[2] = (buffer[19] << 8 | buffer[20]);
                            Send(r);
                        }
                        //else
                        //std::cerr << "Pad packet!" << std::endl;
                    }
                            }
                        }
                    }
                    catch (Socket::WouldBlockException)
                    {
                        boost::this_thread::yield();
                    }
                }
                socket.Send(" ",1);
            }
            
            public:
                ~BTSource()
                {
                    socketThread.interrupt();
                    socketThread.join();
                    socket.Close();
                }
                
                void Open(std::string const &address)
                {
                    socket.Connect(gz4d::BTRFCOMMSocket::Address(address,1));
                    socket.SetNonBlocking();
                    socket.Send("#",1);
                    socketThread = std::move(boost::thread(boost::bind(&BTSource::SocketThread,this)));
                }
                
        };
    }
}

#endif
