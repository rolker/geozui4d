// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
