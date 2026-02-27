/* Copyright (c) 2007 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "SoundOgg.h"
#include <boost/filesystem.hpp>

#include <boost/property_tree/xml_parser.hpp>


REGISTER_OBJECT(SoundOgg, "ogg");

SoundOgg::SoundOgg()
    :Sound()
{
    ok = false;
}

int SoundOgg::Load()
{

    bfs::path fn(GetFilename());
    if(!fn.empty())
    {
        in = fopen(fn.string().c_str(), "rb");
        if(!ov_open(in,&ov_file,NULL,0))
//        if(!ov_fopen(fn.string().c_str(), &ov_file))
        {
            ok = true;
            vorbis_comment * c = ov_comment(&ov_file,-1);
            std::string gz;
            for(int i = 0 ; i < c->comments; i++)
            {
                if(c->comment_lengths[i] > 5)
                {
                    std::string uc = std::string(c->user_comments[i]);
                    if(uc.substr(0,5) == "gz4d=")
                        gz = uc.substr(5);
                }
            }

            std::istringstream ss(gz);
            bpt::ptree doc;
            read_xml(ss,doc);
            
            if(!doc.empty())
            {
                bpt::ptree node = doc.get_child("GZ");
                for(bpt::ptree::const_iterator o = node.begin(); o != node.end(); ++o)
                    if(o->first == "Object")
                    {
                        boost::optional<std::string> attribute = o->second.get_optional<std::string>("<xmlattr>.type");
                        if(attribute && *attribute == "Sound")
                        {
                            LoadXML(o->second,fn.parent_path().string(),gz4d::netcdf::File());
                            if(startTime.valid() && !endTime.valid())
                                endTime = startTime + gz4d::seconds(ov_time_total(&ov_file,-1));
                            else
                                timeSituated = false;
                            bbox.expandTime( startTime.secondsSinceEpoch().asSeconds());
                            bbox.expandTime( endTime.secondsSinceEpoch().asSeconds());
                            return TCL_OK;
                        }
                    }
            }
            else
                timeSituated = false;
            registerLabel();
            return TCL_OK;
        }
    }
    return TCL_ERROR;
}

void SoundOgg::play()
{
    if(ok && !playing)
    {
        if(currentSound)
            currentSound->stop();
        currentSound = this;
        playing = 1;
        if (!timeSituated)
            startTime = gz4d::Time(lastTime);
        soundTime = gz4d::seconds(0.0);
    }
}

// version called by video recorder
void SoundOgg::fillBuffer(uint8_t * stream, unsigned int len, double &t, double timescale)
{
    if(playing && ok)
    {
        ov_time_seek(&ov_file,t-startTime.secondsSinceEpoch().asSeconds());
        vorbis_info * vi = ov_info(&ov_file,-1);

        int l = ((int)(len/4*(vi->rate/44100.0)))*vi->channels*2;
        char* data = new char[l];
        int bs;
        long total = 0;
        while(total < l)
        {
            long n = ov_read(&ov_file,&data[total],l-total,0,2,1,&bs);
            if(n)
                total += n;
            else
                l = total;
        }
        float j;
        for(unsigned int i = 0; i*4 < len; i++)
        {
            j = i*vi->rate/44100.0;
            int ji = (int)j;
            float jr = j-ji;
            ji *= 2*vi->channels;
            if(ji+2*vi->channels < l)
            {
                *(short*)(&stream[i*4]) = (short)(*(short*)(&data[ji])*(1.0-jr) + *(short*)(&data[ji+2*vi->channels])*jr);
                if(vi->channels > 1)
                    *(short*)(&stream[i*4+2]) = (short)(*(short*)(&data[ji+2])*(1.0-jr) + *(short*)(&data[ji+2*vi->channels+2])*jr);
                else
                    memcpy(&stream[i*4+2],&stream[i*4],2);
            }
            else
            {
                memset(&stream[i*4],0,len-(i*4));
                i = len;
            }
        }
        t+=(timescale*len/(4.0*44100.0));
    }
    else
    {
        memset(stream,0,len);
        t+=(timescale*len/(4.0*44100.0));
    }
}


// version called by SDL hook
void SoundOgg::fillBuffer(float * stream, unsigned long len)
{
    if(playing && ok && !recording)
    {
        //std::cerr << "Time: " << lastTime << " Target sound time: " << lastTime-startTime << " Last sound time: " << soundTime << " stream time: " << ov_time_tell(&ov_file) << std::endl;

        if(lastTime-startTime>soundTime+gz4d::seconds(.5) || lastTime-startTime < soundTime-gz4d::seconds(.5))
        {
            if (ov_time_seek(&ov_file,(lastTime-startTime).asSeconds()))
            {
                memset(stream,0,len*2*sizeof(float));
                soundTime = gz4d::seconds(0.0);
                return;
            }
        }
        soundTime = gz4d::seconds(ov_time_tell(&ov_file));

        vorbis_info * vi = ov_info(&ov_file,-1);

        int l = ((int)(len*(vi->rate/44100.0)))*vi->channels;
        std::vector<float> data(l);
        int bs;
        long total = 0;
        while(total < l)
        {
            float **pcm;
            long n = ov_read_float(&ov_file,&pcm,l-total,&bs);
            if(n)
            {
                for(int i = 0; i < n; ++i)
                    for(int c = 0; c < vi->channels; ++c)
                        data[total+i*vi->channels+c] = pcm[c][i];
                total += n*vi->channels;
            }
            else
                l = total;
        }
        float j;
        for(unsigned int i = 0; i*2 < len; i++)
        {
            j = i*vi->rate/44100.0;
            int ji = (int)j;
            float jr = j-ji;
            ji *= vi->channels;
            if(ji+vi->channels < l)
            {
                stream[i*2] = data[ji]*(1.0-jr) + data[ji+vi->channels]*jr;
                if(vi->channels > 1)
                    stream[i*2+1] = data[ji+1]*(1.0-jr) + data[ji+vi->channels+1]*jr;
                else
                    stream[i*2+1] = stream[i*2];
            }
            else
            {
                memset(&stream[i*2],0,(len-(i*2))*2*sizeof(float));
                i = len;
            }
        }
        soundTime+= gz4d::seconds((len/44100.0));
    }
    else
        memset(stream,0,len);
}



