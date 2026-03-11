// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sonar/RDI/FileSource.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/vector/ios.h>
#include <gz4d/nav/Track.h>
#include <gz4d/data/NMEA.h>
#include <gz4d/geo/LocalENU.h>
#include <iostream>
#include <algorithm>
#include <gz4d/time/ios.h>
#include <gz4d/ios.h>

int main(int argc, char *argv[])
{
    std::ifstream nav(argv[1]);
    if(!nav.is_open())
    {
        std::cerr << "Error opening nav" << std::endl;
        exit(1);
    }

    gz4d::nav::Track navTrack;
    gz4d::Time gpstime;
    std::cerr << "Loading nav..." << std::endl;
    while(!nav.eof())
    {
        std::string nmeastr;
        nav >> nmeastr;
        try
        {
            gz4d::nmea::Sentence::Ptr nmea(new gz4d::nmea::Sentence(nmeastr));
            if (nmea->IsType("??RMC"))
            {
                gz4d::nmea::RMC rmc(*nmea);
                if(rmc.year > 0 && rmc.month > 0 && rmc.day > 0 && rmc.hour >= 0 && rmc.minute >= 0 && !gz4d::IsNan(rmc.second))
                    gpstime = gz4d::Time(rmc.year,rmc.month,rmc.day,rmc.hour,rmc.minute,rmc.second);
            }
            if (gpstime.valid())
                navTrack.Consume(std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr>(gpstime,nmea));
        }
        catch (...)
        {
        }
    }

    std::cerr << "Done loading nav: " << navTrack.FirstGPSTime() << " - " << navTrack.LastGPSTime() << std::endl;

    gz4d::Time startTime;
    gz4d::geo::LocalENU<> geoReference;

    for (int i = 2; i < argc; ++i)
    {
        gz4d::ThreadedQueue<gz4d::RDI::Ping::Ptr>::Ptr pingQueue(new gz4d::ThreadedQueue<gz4d::RDI::Ping::Ptr>);

        gz4d::RDI::Ping::Ptr pendingPing;

        gz4d::RDI::FileSource fs(argv[i],2009);
        std::cerr << argv[i] << std::endl;
        std::cerr << fs.GetInfo() << std::endl;

        fs.AddOutQueue(pingQueue);
        while(fs.ProcessNextEnsemble()||!pingQueue->empty())
        {
            if(!pingQueue->empty())
            {
                pendingPing = pingQueue->pop();
                gz4d::nav::Fix::Ptr fix = navTrack(pendingPing->Timestamp());
                if (!gz4d::IsNan(fix->latitude) && !gz4d::IsNan(fix->longitude))
                {
                    //std::cout << fix->timestamp << "," << fix->latitude << "," << fix->longitude << std::endl;

                    float sog = navTrack.SOG(pendingPing->Timestamp()) * 0.514444444; // knots to m/s
                    float cmg = navTrack.CMG(pendingPing->Timestamp());

                    gz4d::RDI::Velocities v = pendingPing->GetData().GetVelocities((fix->heading+gz4d::nav::Fix::heading_type(45.0)).value(),cmg,sog);
                    int count = v.size();

                    std::string sep = "\t";

                    if(!startTime.valid())
                    {
                        startTime = pendingPing->Timestamp();
                        gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon> gr(fix->latitude.value(),fix->longitude.value(),0.0);
                        geoReference = gz4d::geo::LocalENU<>(gr);
                        std::cout << "StartTime:" << sep << startTime << sep << startTime.secondsSinceEpoch().asSeconds() << std::endl;
                        std::cout << "BaseLatitude:" << sep << fix->latitude << std::endl;
                        std::cout << "BaseLongitude:" << sep << fix->longitude << std::endl;
                        std::cout << "StartDepth:" << sep << v[0].range << std::endl;
                        std::cout << "DepthInterval:" << sep << v[1].range - v[0].range << std::endl;
                        std::cout << "BinCount:" << sep << count << std::endl;
                        std::cout << "Time" << sep << "x" << sep << "y" << sep << "sog" << sep << "cmg" << sep << "heading";
                        for(int i = 0; i < 4; ++i)
                            std::cout << sep << "bt" << i;
                        for(int i = 0; i < count; ++i)
                        {
                            std::cout << sep << "u" << i;
                            std::cout << sep << "v" << i;
                            std::cout << sep << "w" << i;
                            std::cout << sep << "e" << i;
                        }
                        for(int i = 0; i < 4; ++i)
                            for(int j = 0; j < count; ++j)
                                std::cout << sep << "r" << i << "_" << j;

                        std::cout << std::endl;
                    }

                    std::cout << pendingPing->Timestamp() - startTime << sep;
                    gz4d::Point<double> position = geoReference.toLocal(gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF>(gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(fix->latitude.value(),fix->longitude.value(),0.0)));
                    std::cout << position[0] << sep << position[1] << sep << sog << sep << cmg << sep << fix->heading+45.0;

                    gz4d::RDI::BottomTrack bt = pendingPing->GetData().GetBottomTrack();
                    for(int i = 0; i < 4; ++i)
                    {
                        std::cout << sep << bt.beams[i].velocity*pendingPing->GetData().SoundSpeed()/(2.0*pendingPing->GetData().Frequency());
                    }


                    for(int i = 0; i < count; ++i)
                    {
                        gz4d::RDI::VelocityVector vel = v[i];
                        std::cout << sep << vel.vector[0] << sep << vel.vector[1] << sep << vel.vector[2] << sep << vel.error;
                    }
                    for(gz4d::RDI::Ping::channels_type::iterator c = pendingPing->GetChannels().begin(); c != pendingPing->GetChannels().end(); ++c)
                    {
                        for(gz4d::RDI::Channel::container_type::sample_iterator<gz4d::RDI::Channel::container_type::functor_types::velocity_type> d = c->Samples().begin(); d != c->Samples().end(); ++d)
                            std::cout << sep << *d;
                    }

                    std::cout << std::endl;

                }
                pendingPing.reset();
            }
        }
    }
    return 0;
}
