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
    if(argc < 5)
    {
        std::cerr << "usage: rdi2gz4d_current_profiles lat lon in [in2 ...] out.gz4d" << std::endl;
        exit(1);
    }
    double lat = atof(argv[1]);
    double lon = atof(argv[2]);

    std::ofstream outfile(argv[argc-1],std::ios::out|std::ios::binary);
    gz4d::binary::Wrapper<gz4d::binary::Navigation::Position::Geographic> nav;
    nav->latitude = lat;
    nav->longitude = lon;
    nav->timestamp = gz4d::Time();
    
    gz4d::Time lastTime;
    
    for(int i = 3; i < argc-1; ++i)
    {
        gz4d::ThreadedQueue<gz4d::RDI::Ping::Ptr>::Ptr pingQueue(new gz4d::ThreadedQueue<gz4d::RDI::Ping::Ptr>);

        gz4d::RDI::Ping::Ptr pendingPing;

        gz4d::RDI::FileSource fs(argv[i],2009);

        fs.AddOutQueue(pingQueue);

    


        while(fs.ProcessNextEnsemble())
        {
            if(!pingQueue->empty())
            {
                pendingPing = pingQueue->pop();
                if(!lastTime.valid())
                {
                    nav->timestamp = pendingPing->Timestamp();
                    std::cout << "nav: " << nav->timestamp << " (" << nav->latitude << ", " << nav->longitude << ")" << std::endl;
                    outfile.write(reinterpret_cast<const char *>(&(*nav)),nav->size);
                }

                gz4d::binary::Wrapper<gz4d::binary::Sonar::CurrentProfile> profile;

                gz4d::RDI::Velocities v = pendingPing->GetData().GetVelocities();
                profile->count = v.size();
                profile.Resize();

                profile->timestamp = pendingPing->Timestamp();
                lastTime = pendingPing->Timestamp();

                gz4d::RDI::PingData::Config config = pendingPing->GetData().getConfig();
                if(config.valid)
                {
                    float transducerDepth = pendingPing->GetData().GetTransducerDepth();
                    for(int i = 0; i < profile->count; ++i)
                    {
                        gz4d::RDI::VelocityVector vel = v[i];
                        profile->vectors[i].depth = -transducerDepth;
                        if(config.transducerDownLooking)
                            profile->vectors[i].depth -= vel.range;
                        else
                            profile->vectors[i].depth += vel.range;
                        profile->vectors[i].error = vel.error;
                        profile->vectors[i].vector.x = vel.vector[0];
                        profile->vectors[i].vector.y = vel.vector[1];
                        profile->vectors[i].vector.z = vel.vector[2];
                    }
                    outfile.write(reinterpret_cast<const char *>(&(*profile)),profile->size);
                }
                pendingPing.reset();
            }
        }
    }
    nav->timestamp = lastTime;
    std::cout << "nav: " << nav->timestamp << " (" << nav->latitude << ", " << nav->longitude << ")" << std::endl;
    outfile.write(reinterpret_cast<const char *>(&(*nav)),nav->size);
    return 0;
}
