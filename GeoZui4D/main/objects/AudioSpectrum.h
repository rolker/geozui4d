/* Copyright (c) 2008 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef AUDIOSPECTRUM_H
#define AUDIOSPECTRUM_H

#include "engine/Object.h"
#include <gz4d/glo/Texture.h>
#include <gz4d/dsp/DiscreetFourierTransform.h>
#include <gz4d/sound/Portaudio.h>
#include <boost/thread/mutex.hpp>
#ifndef WIN32
#include <gz4d/sound/Jack.h>
#endif

class AudioSpectrum : public Object
{
    std::shared_ptr<gz4d::glo::Texture> texture;
    gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> > raster;
    gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> > avgRaster;

    gz4d::DiscreetFourierTransform<double> dft;
    std::size_t N;

    // pulse audio
    gz4d::portaudio::Context paContext;
    gz4d::portaudio::Device paDevice;
    gz4d::portaudio::Stream stream;

#ifndef WIN32
    // jack
    gz4d::jack::Client::Ptr jack;
#endif

    void OpenPulseAudio(uint32_t sampleRate);
    void OpenJack();
    void Open();

    std::vector<std::complex<double> > frequencyDomain;
    std::vector<double> frequencyDomainAverages;
    std::vector<double> timeDomain;
    gz4d::ThreadedQueue<gz4d::AudioPacket::Ptr>::Ptr dataQueue;
    boost::mutex tdMutex;
    double fs;
    unsigned long tindex;
    unsigned long findex;
    int fft_overlap;
    int history;
    public: 
        AudioSpectrum();
        DERIVE_OBJECT(AudioSpectrum);

        void DrawLowRes(float degradation, Window3D * win);
        void Idle(double time);
};

#endif
