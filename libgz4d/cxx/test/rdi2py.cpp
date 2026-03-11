// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sonar/RDI/FileSource.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/vector/ios.h>
#include <gz4d/data/binary/Wrapper.h>
#include <iostream>
#include <algorithm>

template <typename T> void coutFloat(T f)
{
    if(gz4d::IsNan(f))
        std::cout << "None";
    else
        std::cout << f;
}

int main(int argc, char *argv[])
{
    std::cout << "#!/usr/bin/env python\n" << std::endl;
    std::cout << "profiles=(";
    for (int i = 1; i < argc; ++i)
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
                std::cout << "{ 'timestamp':" << std::fixed << pendingPing->Timestamp().secondsSinceEpoch().asSeconds();

                gz4d::RDI::PingData::Config config = pendingPing->GetData().getConfig();
                std::cout << ", 'coordinateFrame':" << config.coordinateFrame;
                std::cout << ", 'valid':" << config.valid;
                std::cout << ", 'frequency':" << config.frequency;
                std::cout << ", 'downLooking':" << config.transducerDownLooking;
                std::cout << ", 'concave':" << config.transducerConcave;
                std::cout << ", 'pingsPerEnsemble':" << pendingPing->GetData().PingsPerEnsemble();
                std::cout << ", 'transducerDepth':" << pendingPing->GetData().GetTransducerDepth();

                std::cout << ",\n        'velocities':(";
                gz4d::RDI::Velocities v = pendingPing->GetData().GetVelocities();
                int count = v.size();
                for(int i = 0; i < count; ++i)
                {
                    gz4d::RDI::VelocityVector vel = v[i];
                    std::cout << "((";
                    coutFloat(vel.vector[0]);
                    std::cout << ", ";
                    coutFloat(vel.vector[1]);
                    std::cout << ", ";
                    coutFloat(vel.vector[2]);
                    std::cout << "), ";
                    coutFloat(vel.range);
                    std::cout << ", ";
                    coutFloat(vel.error);
                    std::cout << "),\n        ";
                }
                std::cout << ")\n    ";
                std::cout << "},\n    ";
                pendingPing.reset();
            }
        }
    }
    std::cout << ")" << std::endl;
    return 0;
}
