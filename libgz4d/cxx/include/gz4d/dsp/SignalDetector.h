// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ_DSP_SIGNALDETECTOR_H
#define GZ_DSP_SIGNALDETECTOR_H

#include <gz4d/gz4d.h>
#include <vector>
#include <memory>
#include <gz4d/util/SignalGenerator.h>

namespace gz4d
{
    class AudioPacket;

    class SignalDetector
    {
        std::vector<float> signal;
        public:
            void init(float frequency, float timeLength, float sampleRate)
            {
                gz4d::SignalGenerator sg;
                sg.SetFrequency(frequency);
                sg.SetSamplingFrequency(sampleRate);
                signal.resize(timeLength*sampleRate);
                sg.SetTimeLength(timeLength);
                sg.FillBuffer(&signal.front(),signal.size());
            }

            float operator()(std::shared_ptr<AudioPacket> ap);
    };
}

#endif
