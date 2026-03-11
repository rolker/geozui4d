// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOUND_PORTAUDIO_DEVICE_H
#define GZ4D_SOUND_PORTAUDIO_DEVICE_H

#include "../Portaudio.h"
#include <gz4d/time/Duration.h>

namespace gz4d
{
    namespace portaudio
    {
        class Handle;
        class Context;
        
        class Device
        {
            friend class Context;
            
            std::shared_ptr<Handle> _handle;
            PaDeviceIndex _index;
            PaDeviceInfo const * _info;
            
            Device(std::shared_ptr<Handle> h, PaDeviceIndex i):_handle(h),_index(i),_info(Pa_GetDeviceInfo(i))
            {
                if(!_info)
                    throw Exception("Device info error");
            }
        public:
            Device(Device const &d):_handle(d._handle),_index(d._index),_info(d._info){}
            operator PaDeviceIndex() const {return _index;}
            std::string Info()
            {
                std::stringstream ret;
                ret << "Name: " << _info->name << "\n";
                ret << "Max no of channels: input: " << _info->maxInputChannels << "\toutput: " << _info->maxOutputChannels << "\n";
                ret << "Default sample rate: " << _info->defaultSampleRate << "\n";
                ret << "Latencies: Input: " << _info->defaultLowInputLatency << " - " << _info->defaultHighInputLatency << "\tOutput: " << _info->defaultLowOutputLatency << " - " << _info->defaultHighOutputLatency << std::endl;
                return ret.str();
            }
            
            std::string getName(){return _info->name;}
            int getMaxInputChannels(){return _info->maxInputChannels;}
            int getMaxOutputChannels(){return _info->maxOutputChannels;}
            time::Duration getDefaultHighInputLatency(){return time::Duration(_info->defaultHighInputLatency);}
            time::Duration getDefaultHighOutputLatency(){return time::Duration(_info->defaultHighOutputLatency);}
            time::Duration getDefaultLowInputLatency(){return time::Duration(_info->defaultLowInputLatency);}
            time::Duration getDefaultLowOutputLatency(){return time::Duration(_info->defaultLowOutputLatency);}
            double getDefaultSampleRate() const {return _info->defaultSampleRate;}
        };
        
    }
}



#endif
