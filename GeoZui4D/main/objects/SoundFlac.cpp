/* Copyright (c) 2007 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "SoundFlac.h"
#include <boost/filesystem.hpp>
//#include <Xml.h>

REGISTER_OBJECT(SoundFlac, "flac");

int SoundFlac::Load()
{

    if(!GetFilename().empty())
    {
        flac.set_filename(GetFilename().string().c_str());
        flac.set_metadata_respond_application((FLAC__byte const *)"gz4d");
        if (flac.init()==FLAC__FILE_DECODER_OK)
        {
            flac.process_until_end_of_metadata();
            Xml xml(flac.getGzx());
            if(xml.docName()=="GZ")
            {
                xml.childNode();
                while(xml.hasCurrentNode())
                {
                    if(xml.nodeName() == "Object")
                    {
                        std::string s;
                        if (xml.getString(s,"type") && s == "Sound")
                        {
                            LoadXML(xml,GetFilename().branch_path().directory_string(),NULL);
                            flac.process_single(); // this is to initialize the stream so sample_rate gets initialized.
                            flac.seek_absolute(0);
                            if(startTime && !endTime)
                                endTime = startTime + flac.getTotalSamples()/ flac.get_sample_rate();
                            bbox.expandTime( startTime);
                            bbox.expandTime( endTime);
                            return TCL_OK;
                        }
                    }
                }
            }
            return TCL_OK;
        }
    }
    return TCL_ERROR;
}

void SoundFlac::play()
{
    if(flac.is_valid() && !playing)
    {
        currentSound->stop();
        currentSound = this;
        playing = 1;
        flac.seek_absolute(0);
        soundTime = 0.0;
    }
}

// version called by SDL hook
void SoundFlac::fillBuffer(Uint8 * stream, unsigned int len)
{
    if(playing && flac.is_valid() && !recording)
    {
        if(lastTime-startTime>soundTime+.5 || lastTime-startTime < soundTime-.5)
        {
            long int pos = (long int)((lastTime-startTime)*flac.get_sample_rate());
            flac.seek_absolute(pos);
            soundTime = lastTime-startTime;
        }
        flac.fillBuffer(stream,len);
        soundTime+=(len/(4.0*44100.0));
    }
    else
        memset(stream,0,len);
}

// GZFlac implementation

SoundFlac::GZFlac::GZFlac()
{
}

FLAC__StreamDecoderWriteStatus SoundFlac::GZFlac::write_callback (const::FLAC__Frame *frame, const FLAC__int32 *const buffer[])
{
    float ratio = 44100/(float)frame->header.sample_rate;
    SampleChunk c;
    c.length = (unsigned int)(frame->header.blocksize*ratio);
    c.data = new Uint8[4*c.length];

    int b=0; // in case data source has only one channel...
    if (frame->header.channels >= 2)
        b=1; // we have a second channel, so set b to it's index
    for(unsigned int i=0; i<c.length; i++)
    {
        ((short*)c.data)[i*2] = (short)(buffer[0][(unsigned int)(i/ratio)]);
        ((short*)c.data)[i*2+1] = (short)(buffer[b][(unsigned int)(i/ratio)]);
    }
    chunks.push(c);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void SoundFlac::GZFlac::metadata_callback(const ::FLAC__StreamMetadata *metadata)
{
    if(metadata->type == FLAC__METADATA_TYPE_APPLICATION)
    {
        char appid[5];
        appid[4] = 0;
        memcpy(appid,metadata->data.application.id,4);
        if(std::string(appid) == "gz4d")
            gzx = (const char *)(metadata->data.application.data);
    }

    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        total_samples = metadata->data.stream_info.total_samples;
    }
}

void SoundFlac::GZFlac::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
}


bool SoundFlac::GZFlac::seek_absolute(FLAC__uint64 sample)
{
    // get rid of our buffered data...
    while(!chunks.empty())
    {
        delete[] chunks.front().data;
        chunks.pop();
    }
    return FLAC::Decoder::File::seek_absolute(sample);
}


bool SoundFlac::GZFlac::fillBuffer(Uint8 * stream, unsigned int len)
{
    unsigned int cursor = 0;
    unsigned int l = len/4;
    while (cursor < l)
    {
        while (chunks.empty())
            process_single();
        if(chunks.front().length <= l-cursor)
        {
            memcpy(&stream[cursor*4],chunks.front().data,chunks.front().length*4);
            cursor += chunks.front().length;
            delete[] chunks.front().data;
            chunks.pop();
        }
        else
        {
            memcpy(&stream[cursor*4],chunks.front().data,(l-cursor)*4);
            for(unsigned int i = 0; i < chunks.front().length-(l-cursor);i++)
                memcpy(&chunks.front().data[i*4],&chunks.front().data[(i+(l-cursor))*4],4);
            chunks.front().length -= (l-cursor);
            cursor = l;
        }
    }
    return true;
}
