// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/ais/SocketListener.h>
#include <gz4d/socket/TCPSocket.h>
#include <gz4d/time/ios.h>
#include <gz4d/ais/History.h>

int main(int argc, char *argv[])
{
    std::string host = "datalogger1.ccom.nh";
    int port = 31414;
    
    if (argc >= 2)
        host = argv[1];
    if (argc >= 3)
        port = atoi(argv[2]);
    
    gz4d::IPSocket::Address a(host,port);
    gz4d::TCPSocket s;
    s.Connect(a);
    gz4d::ais::SocketListener l(s,true);
    gz4d::ais::SocketListener::output_queue_type q = l.NewOutQueue();

    gz4d::ais::History history;
    
    gz4d::time::Duration reportInterval(10);
    gz4d::Time lastReport = gz4d::time::Now();

    std::set<int> whale_notice_sources;
    
    while(true)
    {
        if(!q->empty())
        {
            gz4d::ais::Message m = q->pop();
            if(!history.insert(m))
            {
                //std::cerr << "skipped message with id " << m._message->message_id << " and mmsi " << m._message->mmsi << std::endl;
                //std::cerr << "non-positive mmsi: " << m._message->mmsi << " msg id: " << m._message->message_id << //std::endl;
            }
            if(m._message->message_id == 8)
            {
                std::shared_ptr<Ais8> ais8 = std::dynamic_pointer_cast<Ais8>(m._message);
                
                if(ais8->dac == 1 && ais8->fi == 22)
                {
                    whale_notice_sources.insert(ais8->mmsi);
                    std::shared_ptr<Ais8_001_22> area_notice = std::dynamic_pointer_cast<Ais8_001_22>(m._message);
                    std::cerr << "Rec: " << m._receiver << " repeat: " << area_notice->repeat_indicator << " src: " << area_notice->mmsi << std::endl;
                    std::cerr << "Area notice type: " << area_notice->notice_type << std::endl;
                    std::vector<AisPosition> positions = convertToPositions(*area_notice);
                    for (std::vector<AisPosition>::iterator p = positions.begin(); p != positions.end(); ++p)
                        std::cerr << "\t" << p->longitude << ", " << p->latitude << std::endl;
                    area_notice->print();
                }
                else if(whale_notice_sources.count(ais8->mmsi))
                {
                    std::cerr << ais8->mmsi << "\tdac: " << ais8->dac << " fi: " << ais8->fi << std::endl;
                }
            }
        }
        gz4d::Time now = gz4d::time::Now();
        if(now-lastReport >= reportInterval)
        {
            std::cerr << history.report() << std::endl;
            lastReport = now;
        }
        
        boost::this_thread::yield();
    }
    return 0;
}
