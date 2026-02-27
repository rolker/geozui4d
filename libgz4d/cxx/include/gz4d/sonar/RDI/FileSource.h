#ifndef GZ4D_SONAR_RDI_FILESOURCE_H
#define GZ4D_SONAR_RDI_FILESOURCE_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Sonar.h>
#include <gz4d/sonar/RDI/FileFormat.h>
#include <gz4d/sonar/RDI/ChannelData.h>
#include <gz4d/sonar/RDI/PingData.h>
#include <gz4d/time.h>
#include <gz4d/data/Binary.h>
#include <gz4d/data/NMEA.h>
#include <gz4d/thread/Producer.h>
#include <fstream>
#include <gz4d/util/Swap.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <gz4d/time/ios.h>


namespace gz4d
{
    namespace RDI

    {
        class FileSource: public Producer<Ping::Ptr>
        {
            std::ifstream in;
            float totalTransmitDuration;
            int year;
            std::map<std::size_t,gz4d::Time> log;
            gz4d::Time currentEnsembleTime;
            public:
                FileSource(std::string const &fn, int year)
                :totalTransmitDuration(0.0),year(year)
                {
                    
                    in.open(fn.c_str(),std::ios::in|std::ios::binary);
                    if (!in.is_open())
                        throw Exception("File open error: "+fn);
                    
                    std::ifstream logfile;
                    std::string logfilename = fn+".log";
                    logfile.open(logfilename.c_str());
                    if(logfile.is_open())
                    {
                        double day;
                        std::size_t start,size;
                        int dummy;
                        float dummy2, dummy3;
                        while(!logfile.eof())
                        {
                            logfile >> day >> start >> size >> dummy >> dummy2 >> dummy3;
                            //std::cerr << gz4d::Time(year,day) << "\t" << start << "\t" << size << std::endl;
                            log[start] = gz4d::Time(year,day);
                        }
                    }
                }
                

                EnsembleWrapper GetNextEnsemble()
                {
                    std::shared_ptr<std::vector<char> > ret(new std::vector<char>(2));
                    if(!log.empty())
                        currentEnsembleTime = log[in.tellg()];
                    
                    in.read(&ret->front(),2);
                    if (in.gcount() != 2)
                    {
                        if(in.eof())
                        {
                            in.clear();
                        }
                        return EnsembleWrapper();
                    }
                    EnsembleWrapper ensemble(ret);
                    if(ensemble.old())
                    {
                        ensemble.old()->header.EnsembleSize = Swap(ensemble.old()->header.EnsembleSize);
                        ret->resize(ensemble.old()->header.EnsembleSize +2);
                        in.read(&ret->front()+2,ensemble.old()->header.EnsembleSize);
                        if (in.gcount() != ensemble.old()->header.EnsembleSize)
                        {
                            return EnsembleWrapper();
                        }
                        return ensemble;
                    }
                    ret->resize(4);
                    in.read(&ret->front()+2,2);
                    if(in.gcount() != 2)
                        return EnsembleWrapper();
                    ret->resize(ensemble.GetHeader()->EnsembleSize+2);
                    in.read(&ret->front()+4,ensemble.GetHeader()->EnsembleSize-2);
                    if (in.gcount() != ensemble.GetHeader()->EnsembleSize-2)
                    {
                        return EnsembleWrapper();
                    }
                    return ensemble;
                }
                
                bool ProcessNextEnsemble()
                {
                    EnsembleWrapper packet = GetNextEnsemble();
                    if (packet)
                    {
                        PingData pd(packet,year,currentEnsembleTime);
                        Ping::Ptr ping(new Ping(pd,currentEnsembleTime));
                        if(currentEnsembleTime.valid())
                            ping->SetTimestamp(currentEnsembleTime);
                        else
                            ping->SetTimestamp(ping->GetData().Timestamp());
                        Send(ping);
                        //Producer<std::shared_ptr<file::Ping> >::Send(ping);
                
                        return true;
                    }
                    return false;
                }
                        
                std::size_t FilePosition()
                {
                    return in.tellg();
                }
                

                std::string GetInfo()
                {
                    std::stringstream ret;
                    if(!log.empty())
                        ret << log.begin()->second << " - " << log.rbegin()->second << std::endl;
                    return ret.str();
                }
                
                gz4d::Time GetCurrentEnsembleTime() const
                {
                    return currentEnsembleTime;
                }
                
        };
    }
}

#endif
