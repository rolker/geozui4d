// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_RDI_CHANNELDATA_H
#define GZ4D_SONAR_RDI_CHANNELDATA_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Sonar.h>
#include <gz4d/sonar/RDI/Data.h>
#include <gz4d/sonar/RDI/Sample.h>
#include <gz4d/sonar/Channel.h>
#include <boost/iterator/iterator_adaptor.hpp>
#include <gz4d/util/Swap.h>


namespace gz4d
{
    namespace RDI
    {
            class ChannelData: public Data
            {
                uint8_t channel;


                public:
                    typedef std::shared_ptr<ChannelData> Ptr;
                    typedef SampleAdaptors<ChannelData> functor_types;
                    class iterator : public boost::iterator_adaptor<iterator,uint8_t*>
                    {
                        public:
                            iterator(){}
                            iterator(uint8_t *i):boost::iterator_adaptor<iterator,uint8_t*>(i){}
                            void advance(super_t::difference_type n) { base_reference() += n*4; }
                            void increment() { base_reference() +=4; }
                            void decrement() { base_reference() -=4; }
                            super_t::difference_type distance_to(iterator const& y) const { return (y.base() - base())/4; }
                    };

                    ChannelData()
                    {
                    }
                    
                    ChannelData(EnsembleWrapper ensemble, int year, uint8_t channel, gz4d::Time t)
                    :Data(ensemble,year,t),channel(channel)
                    {
                    }
                    

                    iterator begin()
                    {
                        if(ensemble.old())
                            return &ensemble.old()->velocity[Swap(ensemble.old()->header.VelocitySize)+Swap(ensemble.old()->header.SpectralSize)+channel];
                        return &(ensemble.GetData<EchoIntensityData>()->cell[0].EchoIntensity[channel]);
                    }
                    
                    iterator end()
                    {
                        if(ensemble.old())
                            return &ensemble.old()->velocity[Swap(ensemble.old()->header.VelocitySize)+Swap(ensemble.old()->header.SpectralSize)+Swap(ensemble.old()->header.EchoSize)+channel];
                        return &(ensemble.GetData<EchoIntensityData>()->cell[ensemble.GetData<FixedLeader>()->NumberOfCells].EchoIntensity[channel]);
                    }
                    

                    uint16_t ID() const
                    {
                        return channel;
                    }
                    
                    sonar::BeamGeometry BeamGeometry() const
                    {
                        return Data::BeamGeometry(channel);
                    }
                    
                    float GetVelocity(std::size_t i) const
                    {
                        if(ensemble.old())
                            return DecodeVelocities(&(ensemble.old()->velocity[i*6]))[channel];
                        return DecodeVelocities(ensemble.GetData<VelocityData>()->cell[i].Velocity)[channel];
                    }
                    
                    float GetPercentGood(std::size_t i) const
                    {
                        if(!hasPercentGood())
                            return Nan<float>();
                        if(ensemble.old())
                            return ensemble.old()->velocity[Swap(ensemble.old()->header.VelocitySize)+Swap(ensemble.old()->header.SpectralSize)+Swap(ensemble.old()->header.EchoSize)+i*4+channel];
                        return ensemble.GetData<PercentGoodData>()->cell[i].PercentGood[channel];
                    }
                    
                    uint8_t GetStatus(std::size_t i) const
                    {
                        if(ensemble.old())
                        {
                            uint16_t s = *reinterpret_cast<uint16_t *>(&(ensemble.old()->velocity[Swap(ensemble.old()->header.VelocitySize)+Swap(ensemble.old()->header.SpectralSize)+Swap(ensemble.old()->header.EchoSize)+Swap(ensemble.old()->header.PercentGoodSize)+i*2]));
                            return (s >> 3-i) &0x0f;
                        }
                        return 0;
                    }
                    

            };

            typedef sonar::Channel<ChannelData> Channel;
    }
}
#endif

