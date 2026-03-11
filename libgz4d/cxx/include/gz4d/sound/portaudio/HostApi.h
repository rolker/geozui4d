// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOUND_PORTAUDIO_HOSTAPI_H
#define GZ4D_SOUND_PORTAUDIO_HOSTAPI_H

#include "Exception.h"
#include <sstream>
#include <memory>

namespace gz4d
{
    namespace portaudio
    {
        class Handle;
        class Context;
        
        class HostApi
        {
            friend class Context;

            
            std::shared_ptr<Handle> _handle;
            PaHostApiInfo const * _info;
            
            HostApi(std::shared_ptr<Handle> h, PaHostApiIndex i):_handle(h),_info(Pa_GetHostApiInfo(i))
            {
                if(!_info)
                    throw Exception("HostApi info error");
            }
            
            public:
                HostApi(HostApi const &h):_handle(h._handle),_info(h._info){}
                operator PaHostApiIndex()const{return Pa_HostApiTypeIdToHostApiIndex(_info->type);}
                std::string Info()
                {
                    std::stringstream ret;
                    ret << "Name: " << _info->name << "\n";
                    ret << "No of devices: " << _info->deviceCount << "\n";
                    //ret << "Default input device:" << Device(info->defaultInputDevice).GetName() << "\n";
                    //ret << "Default output device:" << Device(info->defaultOutputDevice).GetName() << "\n";
                    return ret.str();
                }
                
                std::string GetName()
                {
                    return _info->name;
                }
                
                int GetDeviceCount()
                {
                    return _info->deviceCount;
                }
        };
    }
}

#endif
