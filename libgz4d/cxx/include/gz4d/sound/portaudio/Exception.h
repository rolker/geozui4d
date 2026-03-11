// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOUND_PORTAUDIO_EXCEPTION_H
#define GZ4D_SOUND_PORTAUDIO_EXCEPTION_H

#include <gz4d/Exception.h>
#include <portaudio.h>

namespace gz4d
{
    namespace portaudio
    {
        class Exception: public gz4d::Exception
        {
        public:
            Exception(std::string msg):gz4d::Exception(msg){}
            Exception(PaError e):gz4d::Exception(Pa_GetErrorText(e)){}
        };
    }
}



#endif
