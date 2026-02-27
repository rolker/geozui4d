/* Copyright (c) 2009 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef SOUNDDTAG_H
#define SOUNDDTAG_H

#include "Sound.h"
#include <gz4d/dtag.h>
#include <gz4d/dtag/MultiAudioStream.h>
#include <gz4d/dsp/DiscreetFourierTransform.h>

class SoundDtag: public Sound
{
public:
    SoundDtag():si(0),ei(0),dt(0.0125),tspan(10.0),n(256){}
    DERIVE_OBJECT(SoundDtag);

    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

    void play();
    void fillBuffer(float * stream, unsigned long len);
    void fillBuffer(std::vector<int16_t> &buffer, double &t, double timescale);
    void Idle(double time);
    void DrawOrtho();
private:
    gz4d::dtag::DTAG::Ptr dtag;
    gz4d::dtag::MultiAudioStream::Ptr stream;
    std::vector<gz4d::AudioPacket::Ptr> p;
    int cursor;
    std::map<uint32_t,std::vector<double> > spectrum;
    gz4d::DiscreetFourierTransform<float> dft;
    uint32_t si,ei;
    float dt;
    float tspan;
    uint32_t n;
    boost::mutex mutex;

};
#endif
