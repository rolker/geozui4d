// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIGNALGENERATOR_H
#define GZ4D_SIGNALGENERATOR_H

#include <gz4d/gz4d.h>
#include <vector>
#include <cmath>
#include <cstring>

namespace gz4d
{
    /// Produces a sine wave of given amplitude, frequency and sampling rate.
    /// \ingroup utilities
    class SignalGenerator
    {
        public:
            SignalGenerator():frequency(1.0),amplitude(1.0),timeLength(1.0),samplingFrequency(0.0),phase(0.0){}

            double GetFrequency(){return frequency;}
            void SetFrequency(double f){frequency = f; Generate();}

            double GetAmplitude(){return amplitude;}
            void SetAmplitude(double a){amplitude = a; Generate();}

            double GetSamplingFrequency(){return samplingFrequency;}
            void SetSamplingFrequency(double sf){samplingFrequency = sf; Generate();}

            double GetTimeLength(){return timeLength;}
            void SetTimeLength(double tl){timeLength = tl; Generate();}
            
            void FillBuffer(float * buffer, unsigned long size)
            {
                if(data.size() >0)
                {
                    unsigned long total = 0;
                    do
                    {
                        unsigned long startSample = phase;
                        unsigned long copySize = size-total;
                        if (copySize > data.size()-startSample)
                            copySize = data.size()-startSample;
                        memcpy(&buffer[total],&data[startSample],sizeof(float)*copySize);
                        total += copySize;
                        phase = fmod(phase+copySize,samplingFrequency/frequency);
                    }
                    while (total<size);
                }
                else
                    memset(buffer,0,sizeof(float)*size);
            }
            
            template <typename T> void FillContainer(T &container)
            {
                container.insert(container.end(),data.begin(),data.end());
            }
        private:
            inline void Generate()
            {
                if(frequency!=0 && timeLength > 0.0 && samplingFrequency > 0.0)
                {
                    data.resize(samplingFrequency*timeLength);
                    double framesPerCycle = samplingFrequency/frequency;
                    for (unsigned long i = 0; i < data.size(); i++)
                        data[i] = amplitude*::sin((2.0*M_PI*double(i))/framesPerCycle);
                }
                else
                    data.clear();
            }

            double frequency;
            double samplingFrequency;
            double timeLength;
            double amplitude;

            double phase;

            std::vector<float> data;
    };
}

#endif
