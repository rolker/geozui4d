// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sonar/RDI/FileSource.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/vector/ios.h>
#include <gz4d/data/binary/Wrapper.h>
#include <iostream>
#include <algorithm>
#include <gz4d/time/ios.h>

int main(int argc, char *argv[])
{
#ifndef WIN32
    for (int i = 1; i < argc; ++i)
    {
        gz4d::ThreadedQueue<gz4d::RDI::Ping::Ptr>::Ptr pingQueue(new gz4d::ThreadedQueue<gz4d::RDI::Ping::Ptr>);

        gz4d::RDI::Ping::Ptr pendingPing;

        gz4d::RDI::FileSource fs(argv[i],2009);
        std::cout << argv[i] << std::endl;
        std::cout << fs.GetInfo() << std::endl;

        fs.AddOutQueue(pingQueue);
        while(fs.ProcessNextEnsemble())
        {
            if(!pingQueue->empty())
            {
                pendingPing = pingQueue->pop();
                std::cout << pendingPing->Timestamp() << std::endl;
                std::cout << pendingPing->GetData().Info();
                gz4d::RDI::PingData::Config config = pendingPing->GetData().getConfig();
                std::cout << "\tcoordinate frame (0-beam,1-inst,2-ship,3-earth): " << config.coordinateFrame << std::endl;
                std::cout << "\tvalid: " << config.valid << std::endl;
                std::cout << "\tfrequency: " << config.frequency << std::endl;
                std::cout << "\tdown looking: " << config.transducerDownLooking << std::endl;
                std::cout << "\tconcave: " << config.transducerConcave << std::endl;
                std::cout << "\t" << pendingPing->GetData().PingsPerEnsemble() << " pings per ensemble" << std::endl;
                std::cout << "\tsample interval: " << pendingPing->GetChannels()[0].GetSampleInterval() << " s, " << pendingPing->GetChannels()[0].GetSampleInterval()*pendingPing->GetData().SoundSpeed()/2.0 << " m"<< std::endl;
                
                std::cout << "\tTransducer depth: " << pendingPing->GetData().GetTransducerDepth() << std::endl;
                gz4d::RDI::BottomTrack bt = pendingPing->GetData().GetBottomTrack();
                for(int i = 0; i < 4; ++i)
                {
                    std::cout << "\t" << i << " velocity: " << bt.beams[i].velocity << "\trange: " << bt.beams[i].range << "\t%good: " << bt.beams[i].percentGood << std::endl;
                }

                gz4d::RDI::Velocities v = pendingPing->GetData().GetVelocities();
                int count = v.size();
                std::cout << "\t" << count << " samples" << std::endl;
                bool beamCoordinates =  pendingPing->GetData().getConfig().coordinateFrame == gz4d::RDI::Data::Config::Beam_Coordinates;
                for(int i = 0; i < count && i < 15; ++i)
                {
                    gz4d::RDI::VelocityVector vel = v[i];
                    std::cout << "\t" << vel.vector << "\trange: " << vel.range << "\terror: " << vel.error;
                    if(!beamCoordinates)
                        std::cout << "\txy magnitude: " << sqrt(vel.vector[0]*vel.vector[0]+vel.vector[1]*vel.vector[1]) << " heading: " << gz4d::Degrees(atan2(vel.vector[0],vel.vector[1]));
                    std::cout << std::endl;
                }

                for(gz4d::RDI::Ping::channels_type::iterator c = pendingPing->GetChannels().begin(); c != pendingPing->GetChannels().end(); ++c)
                {
                    if(!c->Samples().empty())
                    {
                        std::cout << "\t" << c->ID() << "\t";
                        int count = 0;
                        for(gz4d::RDI::Channel::container_type::sample_iterator<gz4d::RDI::Channel::container_type::functor_types::velocity_type> d = c->Samples().begin(); d != c->Samples().end() && count < 15; ++d, ++count)
                            std::cout << *d << ", ";
                        std::cout << std::endl;
                    }
                }

                std::cout << std::endl;
                pendingPing.reset();
            }
        }
    }
#endif
    return 0;
}
