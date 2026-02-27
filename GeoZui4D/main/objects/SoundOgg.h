/* Copyright (c) 2007 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef SOUNDOGG_H
#define SOUNDOGG_H

#include "Sound.h"

#include <vorbis/vorbisfile.h>

class SoundOgg: public Sound
{
public:
    SoundOgg();

    DERIVE_OBJECT(SoundOgg);

    int Load();

    void play();
    virtual void fillBuffer(float * stream, unsigned long len);
    virtual void fillBuffer(uint8_t * stream, unsigned int len,double &t, double timescale);
private:
    OggVorbis_File ov_file;
    bool ok;
};
#endif
