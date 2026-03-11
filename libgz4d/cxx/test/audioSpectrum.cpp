// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/gz4d.h>

#include <gz4d/glut/Application.h>
#include <gz4d/sound/AudioSpectrum.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/sound/AudioPacket.h>
#include <gz4d/sound/Portaudio.h>

class GlutSpectrum: public gz4d::AudioSpectrum, public gz4d::glut::Application<GlutSpectrum>
{
    public:
        GlutSpectrum(){}
};

int main(int argc, char *argv[])
{
    std::shared_ptr<GlutSpectrum> gs(new GlutSpectrum);
    
    gz4d::Consumer<GlutSpectrum,gz4d::AudioPacket::Ptr> c(gs);

    gs->SetBins(100,1000,10);
    
    gz4d::portaudio::Context pac;
    gz4d::portaudio::Format f = pac.getDefaultFormat();
    f.getOutputParameters().setChannelCount(0);
    gz4d::portaudio::Stream s = f.open();

    s.AddOutQueue(c.GetInQueue());
    
    s.start();
    
    (*gs)("Spectrum");
}

