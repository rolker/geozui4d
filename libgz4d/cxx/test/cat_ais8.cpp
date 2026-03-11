// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/ais/SocketListener.h>
#include <gz4d/socket/TCPSocket.h>
#include <gz4d/time/ios.h>
#include <gz4d/ais/History.h>

int main(int argc, char *argv[])
{
    std::vector<std::string> sources;
    
    if (argc == 1)
    {
        sources.push_back("tcp://datalogger1.ccom.nh:31414?uscg_nais");
        sources.push_back("tcp://datalogger1.ccom.nh:31415");
    }
    
    for(int i = 1; i < argc; ++i)
        sources.push_back(argv[i]);
    
    
    std::vector<std::shared_ptr<gz4d::ais::SocketListener> > listeners;
    
    gz4d::ais::SocketListener::output_queue_type q;
    
    for(std::vector<std::string>::const_iterator s = sources.begin(); s != sources.end(); ++s)
    {
        std::shared_ptr<gz4d::ais::SocketListener> l(new gz4d::ais::SocketListener(*s));
        if (listeners.empty())
            q = l->NewOutQueue();
        else
            l->AddOutQueue(q);
        listeners.push_back(l);
    }

    while(true)
    {
        if(!q->empty())
        {
            gz4d::ais::Message m = q->pop();
            if(m._message->message_id == 8)
            {
                std::shared_ptr<Ais8> ais8 = std::dynamic_pointer_cast<Ais8>(m._message);
                if(!(ais8->dac == 366 && (ais8->fi == 56 || ais8->fi == 57)))
                {
                    std::cout << m._timestamp << "," << m._payload << "," << ais8->mmsi << "," << ais8->dac;
                    
                    switch(ais8->dac)
                    {
                        case 1:
                            std::cout << ",IMO" << "," << ais8->fi;
                            switch(ais8->fi)
                            {
                                case 21:
                                    std::cout << ",Ship WX";
                                    break;
                                case 22:
                                    std::cout << ",Area Notice";
                                    {
                                        std::shared_ptr<Ais8_001_22> area_notice = std::dynamic_pointer_cast<Ais8_001_22>(m._message);
                                        std::cout << ",type=" << area_notice->notice_type << ",link_id=" << area_notice->link_id;
                                        std::vector<AisPosition> positions = convertToPositions(*area_notice);
                                        for (std::vector<AisPosition>::iterator p = positions.begin(); p != positions.end(); ++p)
                                            std::cout << "," << p->longitude << ", " << p->latitude;
                                    }
                                    break;
                                case 24:
                                    std::cout << ",Voyage Ext";
                                    break;
                                case 26:
                                    std::cout << ",Environmental";
                                    break;
                                case 31:
                                    std::cout << ",Met/Hydro";
                                    break;
                            }
                            break;
                        case 200:
                            std::cout << ",Inland" << "," << ais8->fi;
                            switch(ais8->fi)
                            {
                                case 10:
                                    std::cout << ",Voyage";
                                    {
                                        std::shared_ptr<Ais8_200_10> v = std::dynamic_pointer_cast<Ais8_200_10>(m._message);
                                        //v->print();
                                        std::cout << ",id=" << v->vessel_id_eu << ",length=" << v->length << ",beam=" << v->beam;
                                    }
                                    
                            }
                            break;
                        case 219:
                            std::cout << ",Denmark" << "," << ais8->fi;
                            break;
                        case 316:
                            std::cout << ",Canada" << "," << ais8->fi;
                            break;
                        case 366:
                            std::cout << ",USA" << "," << ais8->fi;
                            switch(ais8->fi)
                            {
                                case 22:
                                    std::cout << ",Area Notice";
                                    {
                                        std::shared_ptr<Ais8_366_22> area_notice = std::dynamic_pointer_cast<Ais8_366_22>(m._message);
                                        std::cout << ",type=" << area_notice->notice_type;
                                    }
                                    break;
                            }
                            break;
                            
                        default:
                            std::cout << ",unknown" << "," << ais8->fi;
                    }
                    
                    std::cout << std::endl;
                }
            }
        }
        boost::this_thread::yield();
    }
    return 0;
}
