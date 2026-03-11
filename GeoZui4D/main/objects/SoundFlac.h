// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef SOUNDFLAC_H
#define SOUNDFLAC_H

#include "Sound.h"

#include <FLAC++/decoder.h>

class SoundFlac: public Sound
{
public:
    DERIVE_OBJECT(SoundFlac);

    int Load();

    void play();
    virtual void fillBuffer(Uint8 * stream, unsigned int len);
private:
    class GZFlac: public FLAC::Decoder::File
    {
        public:
            GZFlac();
            std::string const &getGzx(){return gzx;};
            unsigned long getTotalSamples(){return total_samples;};
            bool fillBuffer(Uint8 * stream, unsigned int len);
            bool seek_absolute(FLAC__uint64 sample);
        protected:
            ::FLAC__StreamDecoderWriteStatus write_callback (const::FLAC__Frame *frame, const FLAC__int32 *const buffer[]);
            void metadata_callback(const ::FLAC__StreamMetadata *metadata);
            void error_callback(::FLAC__StreamDecoderErrorStatus status);
            std::string gzx;
            FLAC__uint64 total_samples;

            struct SampleChunk
            {
                Uint8 * data;
                unsigned int length;
            };
            std::queue<SampleChunk> chunks;
    };

    GZFlac flac;
};
#endif
