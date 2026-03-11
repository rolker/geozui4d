// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sonar/RDI/FileSource.h>
#include <gz4d/sonar/BottomDetection.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/vector/ios.h>
#include <gz4d/data/binary/Wrapper.h>
#include <gz4d/nav/Track.h>
#include <gz4d/data/NMEA.h>
#include <gz4d/geo/LocalENU.h>
#include <iostream>
#include <algorithm>
#include <boost/concept_check.hpp>
#include <gz4d/time/ios.h>

int main(int argc, char *argv[])
{
#ifndef WIN32
    std::ifstream nav(argv[1]);
    if(!nav.is_open())
    {
        std::cerr << "Error opening nav" << std::endl;
        exit(1);
    }
    
    gz4d::nav::Track navTrack;
    gz4d::Time gpstime;
    int count = 0;
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
        ++count;
        if(count > 2500)
        {
            std::cerr << "nav: " << navTrack.LastGPSTime() << std::endl;
            count = 0;
        }
    }

    float depth_factor = cos(gz4d::Radians<>(30.0));

    long good = 0;
    long bad = 0;
    long good_nav = 0;
    long bad_nav = 0;

    for (int i = 2; i < argc; ++i)
    {
        gz4d::RDI::FileSource fs(argv[i],2009);
        std::cerr << argv[i] << std::endl;
        std::cerr << fs.GetInfo() << std::endl;

        double avg[4];
        for(int j = 0; j < 4; ++j)
            avg[j]= gz4d::Nan<double>();

        gz4d::RDI::EnsembleWrapper packet = fs.GetNextEnsemble();
        while(packet)
        {
            gz4d::RDI::PingData pd(packet,2009,fs.GetCurrentEnsembleTime());
            gz4d::RDI::BottomTrack bt = pd.GetBottomTrack();
            gz4d::nav::Fix::Ptr fix = navTrack(pd.Timestamp());
            if(!gz4d::IsNan(fix->longitude) && !gz4d::IsNan(fix->latitude) && !gz4d::IsNan(fix->heading))
            {
                ++good_nav;
                gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon> gr = *fix; //(fix->latitude,fix->longitude,0.0);
                gz4d::geo::LocalENU<> geoReference = gz4d::geo::LocalENU<>(gr);

                float ship_heading = fix->heading.value();
                float adcp_heading = ship_heading+45.0;
                //std::cout << pd.Timestamp() <<  "\t" << std::fixed << std::setprecision(6) << fix->longitude << ", " << fix->latitude << "\theading ship: " << ship_heading << " adcp: " << adcp_heading << std::endl;
                float adcp_h_r = gz4d::Radians<>(adcp_heading);
                //if(bt.beams[0].percentGood > 0.0 && bt.beams[1].percentGood > 0.0 && bt.beams[2].percentGood > 0.0 && bt.beams[3].percentGood > 0.0)
                {
                    //std::cout << std::fixed << std::setprecision(6) << double(pd.Timestamp());
                    for (int j = 0; j < 4; ++j)
                    {
                        double range = bt.beams[j].range;


                        gz4d::sonar::BeamGeometry bg = pd.BeamGeometry(j);
                        if (bt.beams[j].percentGood > 0.0)
                        {
                            ++good;
                            if (gz4d::IsNan(avg[j]))
                                avg[j] = range;
                            
                            if (range < avg[j]*.85)
                                range = avg[j];
                            else
                                avg[j] = avg[j]*.5 + range*.5;
                            
                            float x_rel_adcp = range * sin(gz4d::Radians<>(bg.direction[0]));
                            float y_rel_adcp = range * sin(gz4d::Radians<>(bg.direction[1]));
                            float x = cos(adcp_h_r)*x_rel_adcp + sin(adcp_h_r)*y_rel_adcp;
                            float y = -sin(adcp_h_r)*x_rel_adcp + cos(adcp_h_r)*y_rel_adcp;
                            gz4d::Point<double> xyz_local(x,y,-range*depth_factor);
                            gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF> xyz_ecef = geoReference.toECEF(xyz_local);
                            gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon> xyz_latlon(xyz_ecef);
                            //std::cout << j << " - range: " << bt.beams[j].range << " depth: " << bt.beams[j].range*depth_factor << " rx,ry: " << x_rel_adcp << ", " << y_rel_adcp << " xyz local: " << xyz_local << " lat,lon,depth: " << xyz_latlon << std::endl;
                            std::cout << std::fixed << std::setprecision(6) << xyz_latlon[1] << "\t" << xyz_latlon[0] << "\t" << -xyz_latlon[2] << std::endl;
                            //std::cout << std::fixed << std::setprecision(6) << "\t" << xyz_latlon[1] << "\t" << xyz_latlon[0] << "\t" << -xyz_latlon[2];
                        }
                        else
                            ++bad;
                    }
                    //std::cout << std::endl;
                }
            }
            else
                ++bad_nav;
            packet = fs.GetNextEnsemble();
        }
    }
    std::cerr << "good bottom: " << good << "\tbad bottom: " << bad << std::endl;
    std::cerr << "good nav: " << good_nav << "\tbad nav: " << bad_nav << std::endl;
#endif
    return 0;
}
