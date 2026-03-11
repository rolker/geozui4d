// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_RDI_VELOCITIES_H
#define GZ4D_SONAR_RDI_VELOCITIES_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/RDI/Data.h>
#include <gz4d/util/Swap.h>


namespace gz4d
{
    namespace RDI
    {
        struct VelocityVector
        {
            Vector<float,3> vector;
            float error;
            float range;
        };

        class Velocities: public Data
        {

            bool instrumentFrameOfReference;
            BottomTrack bottomTrack;
            float heading;
            Vector<float,3> motion;
            public:
                Velocities(Data const &d)
                :Data(d),instrumentFrameOfReference(true)
                {
                }
                
                Velocities(Data const &d, float heading, float course, float speed)
                :Data(d),instrumentFrameOfReference(false),heading(Radians(heading))
                {
                    bottomTrack = GetBottomTrack();
                    motion = Vector<float,3>(speed*sin(Radians(course)),speed*cos(Radians(course)),0.0);
                }
                

                std::size_t size()
                {
                    if(ensemble.old())
                        return Swap(ensemble.old()->header.VelocitySize)/6;
                    return ensemble.GetData<FixedLeader>()->NumberOfCells;
                }
                
                VelocityVector operator[](std::size_t i)
                {
                    VelocityValues v;
                    if(ensemble.old())
                        v = DecodeVelocities(&(ensemble.old()->velocity[i*6]));
                    else
                        v = DecodeVelocities(ensemble.GetData<VelocityData>()->cell[i].Velocity);
                    
                    if(getConfig().coordinateFrame == Config::Beam_Coordinates)
                    {
                        if(!instrumentFrameOfReference && bottomTrack)
                        {
                            for(int i = 0; i < 4; ++i)
                                v[i] -= bottomTrack.beams[i].velocity;
                        }
                        
                        VelocityVector ret;
                        
                        float vxs = 1.0/(2.0*config.sangle);
                        float vys = vxs;
                        float vzs = 1.0/(4.0*config.cangle);
                        float ves = 1.0/(4.0);
                        
                        
                        ret.vector[0] = vxs * (-v[0]+v[1]);
                        ret.vector[1] = vys * (v[2]-v[3]);
                        ret.vector[2] = vzs * (v[0]+v[1]+v[2]+v[3]);
                        ret.error = ves * (v[0]+v[1]-v[2]-v[3]);
                        
                        ret.range =  config.cangle*(SampleOffset()+i*SampleInterval())*SoundSpeed()/2.0;
                        
                        if(!instrumentFrameOfReference)
                        {
                            ret.vector = Vector<float,3>(ret.vector[0]*cos(heading)+ret.vector[1]*sin(heading),-ret.vector[0]*sin(heading)+ret.vector[1]*cos(heading),ret.vector[2]);
                            if(!bottomTrack)
                                ret.vector += motion;
                        }
                        
                        return ret;
                    }
                    VelocityVector ret;
                    ret.vector[0] = v[0];
                    ret.vector[1] = v[1];
                    ret.vector[2] = v[2];
                    ret.error = v[3];
                    ret.range = getConfig().cangle*(SampleOffset()+i*SampleInterval())*SoundSpeed()/2.0;
                    return ret;
                }
                
        };
    }
}

#endif
