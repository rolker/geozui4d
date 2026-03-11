// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOUND_PORTAUDIO_CONTEXT_H
#define GZ4D_SOUND_PORTAUDIO_CONTEXT_H

#include "../Portaudio.h"
#include "HostApi.h"
#include "Device.h"
#include "Format.h"

#include <memory>

namespace gz4d
{
    namespace portaudio
    {
        
        class Context;
        class Handle
        {
            friend class Context;
            PaError _error;
            Handle()
            :_error(Pa_Initialize())
            {
                if(_error != paNoError)
                    throw Exception(_error);
            }
        public:
            ~Handle()
            {
                Pa_Terminate();
            }
        };
        
        class Context
        {
            
            std::shared_ptr<Handle> _handle;
        public:
            Context():_handle(new Handle){}
            Context(Context const &c):_handle(c._handle){}
            
            int getHostApiCount() const {return Pa_GetHostApiCount();}
            HostApi getHostApi(int i) const {return HostApi(_handle,i);}
            
            HostApi getHostApi(Device const &d) {return HostApi(_handle, d._info->hostApi);}
            
            HostApi getDefaultHostApi() const {return HostApi(_handle, Pa_GetDefaultHostApi());}
            
            int getDeviceCount() const {return Pa_GetDeviceCount();}
            
            Device getDevice(int i) const {return Device(_handle, i);}
            Device getDevice(HostApi const &a, int i) const {return Device(_handle, Pa_HostApiDeviceIndexToDeviceIndex(a, i));}
            
            Device getDefaultInputDevice() const {return Device(_handle,Pa_GetDefaultInputDevice());}
            Device getDefaultOutputDevice() const {return Device(_handle,Pa_GetDefaultOutputDevice());}
            Device getDefaultInputDevice(HostApi const &a) const {return Device(_handle,a._info->defaultInputDevice);}
            Device getDefaultOutputDevice(HostApi const &a) const {return Device(_handle,a._info->defaultOutputDevice);}
            
            Format getDefaultFormat() const 
            {
                Device id = getDefaultInputDevice();
                Device od = getDefaultOutputDevice();
                return Format(_handle,id,id.getDefaultHighInputLatency().asSeconds(),od,od.getDefaultHighOutputLatency().asSeconds(),od.getDefaultSampleRate());
            }
            
        };
    }
}



#endif
