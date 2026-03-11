// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_FILESOURCE_H
#define GZ4D_SIMRAD_FILESOURCE_H

#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/file/FileFormat.h>
#include <gz4d/sonar/Simrad/file/ChannelData.h>
#include <gz4d/time.h>
#include <gz4d/data/Binary.h>
#include <gz4d/data/NMEA.h>
#include <gz4d/thread/Producer.h>
#include <fstream>
#include <boost/property_tree/xml_parser.hpp>

namespace gz4d
{
    namespace Simrad

    {
        struct ConfigurationDatagram;

        class FileSource: protected Producer<std::pair<gz4d::Time, nmea::Sentence::Ptr> >, protected Producer<file::Ping::Ptr>
        {
            std::ifstream in;
            bool me70;
            DatagramWrapper<ConfigurationDatagram> config;
            float totalTransmitDuration;
            
            file::Ping::Ptr ping;
            gz4d::Time lastPingTime;
            
            public:
                FileSource(std::string const &fn)
                :me70(false),totalTransmitDuration(0.0)
                {
                    
                    in.open(fn.c_str(),std::ios::in|std::ios::binary);
                    if (!in.is_open())
                        throw Exception("File open error: "+fn);
                    
                    config = GetNextDatagram();
                    if(config)
                    {
                        if(config->IsValid())
                        {
                            if (std::string(config->ConfigHeader.SounderName) == "MBES")
                            {
                                me70 = true;
                                DatagramWrapper<ConfigurationDatagram1> beamConfig = GetNextDatagram();
                                boost::property_tree::ptree bc;
                                read_xml(beamConfig->BeamConfiguration,bc);
                                boost::property_tree::ptree::const_iterator n = bc.begin();
                                std::cerr << "Beam configuration: " << n->first << std::endl;
                                float pulseDuration = 0.0;
                                int beamGroupSize = 0;
                                bool refBeamsSeparateGroups = false;
                                int beamCount = 0;
                                int refBeamCount = 0;
                                
                                std::pair<boost::property_tree::ptree::const_assoc_iterator,boost::property_tree::ptree::const_assoc_iterator> params = n->second.equal_range("Configuration.Frequency.parameter");
                                for(boost::property_tree::ptree::const_assoc_iterator cn = params.first; cn != params.second; ++cn)
                                {
                                    std::string name;
                                    name = cn->second.get("<xmlattr>.name",name);
                                    if (name == "PulseDuration")
                                        pulseDuration = cn->second.get("<xmlattr>.value",pulseDuration);
                                    if (name == "BeamTxGroupSize")
                                        beamGroupSize = cn->second.get("<xmlattr>.value",beamGroupSize);
                                    if (name == "RefBeamsInSeparateTxGroups")
                                        refBeamsSeparateGroups = cn->second.get("<xmlattr>.value",refBeamsSeparateGroups);
                                }
                                
                                params = n->second.equal_range("Configuration.Fan.parameter");
                                for(boost::property_tree::ptree::const_assoc_iterator cn = params.first; cn != params.second; ++cn)
                                {
                                    std::string name;
                                    name = cn->second.get("<xmlattr>.name",name);
                                    if (name == "NoOfBeamsInFan")
                                        beamCount = cn->second.get("<xmlattr>.value",beamCount);
                                }
                                
                                params = n->second.equal_range("Configuration.RefBeamA.parameter");
                                for(boost::property_tree::ptree::const_assoc_iterator cn = params.first; cn != params.second; ++cn)
                                {
                                    std::string name;
                                    name = cn->second.get("<xmlattr>.name",name);
                                    if (name == "Active")
                                        refBeamCount = cn->second.get("<xmlattr>.value",refBeamCount);
                                }

                                params = n->second.equal_range("Configuration.RefBeamB.parameter");
                                for(boost::property_tree::ptree::const_assoc_iterator cn = params.first; cn != params.second; ++cn)
                                {
                                    std::string name;
                                    name = cn->second.get("<xmlattr>.name",name);
                                    if (name == "Active")
                                        refBeamCount += cn->second.get("<xmlattr>.value",0);
                                }
                                
                                
                                std::cerr << "\tPulse Duration: " << pulseDuration << "\n\tGrp Size: " << beamGroupSize << "\n\tRefs separate: " << refBeamsSeparateGroups << "\n\tBeams: " << beamCount << "\n\tRef Beams: " << refBeamCount << std::endl;
                                
                                int pulses = ceil(beamCount/float(beamGroupSize));
                                if(refBeamsSeparateGroups)
                                    pulses += refBeamCount;
                                else
                                    if(refBeamCount)
                                        ++pulses;
                                    totalTransmitDuration = pulseDuration*pulses;
                                std::cerr << "Total transmit duration: " << totalTransmitDuration << "\t(" << pulses << " pulses)" << std::endl;
                                
                            }
                            
                        }
                        else
                        {
                            std::cerr << "Config not found" << std::endl;
                        }
                    }
                    else
                        std::cerr << "Packet not found" << std::endl;
                    
                }
                

                template<typename T> void AddOutQueue(typename Producer<T>::output_queue_type &q)
                {
                    Producer<T>::AddOutQueue(q);
                }

                DatagramWrapperBase GetNextDatagram()
                {
                    std::shared_ptr<std::vector<char> > ret(new std::vector<char>);
                    int32_t size;
                    in.read(reinterpret_cast<char*>(&size),4);
                    if (in.gcount() != 4)
                    {
                        if(in.eof())
                        {
                            in.clear();
                        }
                        return DatagramWrapperBase();
                    }
                    ret->resize(size);
                    in.read(&ret->front(),size);
                    if (in.gcount() != size)
                    {
                        return DatagramWrapperBase();
                    }
                    
                    uint32_t sizeCheck;
                    in.read(reinterpret_cast<char*>(&sizeCheck),4);
                    if (sizeCheck != size)
                        return DatagramWrapperBase();
                    return DatagramWrapperBase(ret);
                }
                
                bool ProcessNextDatagram()
                {
                    DatagramWrapperBase packet = GetNextDatagram();
                    if (packet)
                    {
                        DatagramWrapper<NMEADatagram> nmea = packet;
                        if(nmea->IsValid())
                        {
                            //std::cerr << nmea->Text << std::endl;
                            nmea::Sentence::Ptr n(new nmea::Sentence(nmea->Text,false));
                            Producer<std::pair<gz4d::Time, nmea::Sentence::Ptr> >::Send(std::pair<gz4d::Time, nmea::Sentence::Ptr> (gz4d::time::FromNT(nmea->DgHeader.DateTime),n));
                            boost::this_thread::yield();
                            return true;
                        }
                        DatagramWrapper<SampleDatagram> sample = packet;
                        if(sample->IsValid())
                        {
                            gz4d::Time t = gz4d::time::FromNT(sample->DgHeader.DateTime);
                            if (t != lastPingTime)
                            {
                                if(lastPingTime.valid())
                                {
                                    Producer<std::shared_ptr<file::Ping> >::Send(ping);
                                    boost::this_thread::yield();
                                }
                                ping = file::Ping::Ptr(new file::Ping(t));
                                ping->GetData().GetChannels().resize(config->ConfigHeader.TransducerCount);
                                lastPingTime = t;
                            }
                            if(!(config->Beam[sample->Channel-1].BeamType & 0x2))
                            {
                                file::ChannelData::Ptr cd(new file::ChannelData(config,sample,totalTransmitDuration));
                                ping->GetData().GetChannels()[sample->Channel - 1] = file::Channel(cd);
                            }
                            return true;
                        }
                        return true;
                    }
                    return false;
                }
                

                bool IsME70()
                {
                    return me70;
                }
                
                
                std::string GetInfo()
                {
                    std::stringstream ret;
                    if(config && config->IsValid())
                    {
                        ret << config->ConfigHeader.SurveyName << "\n" << config->ConfigHeader.TransducerCount << " channels" << "\n";
                        for (int i = 0; i < config->ConfigHeader.TransducerCount; ++i)
                        {
                            ret << '\t' << config->Beam[i].ChannelId << "\n";
                            ret << "\t\t" << config->Beam[i].BeamType << "\t" << config->Beam[i].Frequency << "\t" << config->Beam[i].BeamWidthAlongship << "\t" << config->Beam[i].BeamWidthAthwartship  << "\t" << config->Beam[i].BeamSteeringAngleAlongship << "\t" << config->Beam[i].BeamSteeringAngleAthwartship << "\n";
                        }
                    }
                    else
                    {
                        ret << "Configuration not found" << "\n";
                    }
                    return ret.str();
                }
                
        };
    }
}

#endif
