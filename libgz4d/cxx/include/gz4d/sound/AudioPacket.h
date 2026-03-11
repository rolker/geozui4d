// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOUND_AUDIOPACKET_H
#define GZ4D_SOUND_AUDIOPACKET_H

#include <gz4d/time.h>

namespace gz4d
{
    struct AudioPacket
    {
        Time timestamp;
        uint32_t sample_rate;
        uint32_t sample_number;
        std::vector<float> samples;

        typedef std::shared_ptr<AudioPacket> Ptr;
    };
}

#endif
