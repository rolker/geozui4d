// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "SoundDtag.h"
#include <filesystem>
#include "engine/Scene3D.h"
#include "engine/TimeControl.h"
#include "MainWindow.h"
#include <algorithm>
#include <cmath>
#include <gz4d/glo/PushAttrib.h>
#include "engine/TextureMaps/TextureMap.h"


extern MainWindow *mainWindow;

REGISTER_OBJECT(SoundDtag, "");


Object * SoundDtag::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Sound::LoadXML(xml,path,ncfile);

    float utc_offset =  xml.get<float>("<xmlattr>.utc_offset",0.0);

    boost::optional<std::string> dtagdir = xml.get_optional<std::string>("<xmlattr>.dtagdir");
    if(dtagdir)
    {
        dtag = gz4d::dtag::DTAG::Ptr(new gz4d::dtag::DTAG(*dtagdir));
        dtag->SetTimeOffset(utc_offset);
        stream = gz4d::dtag::MultiAudioStream::Ptr(new gz4d::dtag::MultiAudioStream);
        dtag->GetStream(*stream);
        startTime = dtag->GetStartTime();
        endTime = dtag->GetEndTime();
        bbox.expandTime( startTime.secondsSinceEpoch().asSeconds());
        bbox.expandTime( endTime.secondsSinceEpoch().asSeconds());
        timeSituated = true;
    }
    return this;
}


void SoundDtag::play()
{
    if(stream && !playing)
    {
        if(currentSound)
            currentSound->stop();
        currentSound = this;
        playing = 1;
        soundTime = gz4d::seconds(0.0);


    }
}

void SoundDtag::Idle(double time)
{
    Sound::Idle(time);
    if(dtag)
    {
        gz4d::Time timeout = gz4d::time::Now() + gz4d::seconds(0.1);
        if(fabs(Scene3D::timeControl->getScale())>2.0)
        {
            si = 0;
            ei = 0;
        }
        else
        {
            gz4d::dtag::MultiAudioStream as;
            dtag->GetStream(as);
            double stime = time-startTime.secondsSinceEpoch().asSeconds()-tspan/2.0;
            si = 0;
            if (stime > 0.0)
                si = stime/dt;
            ei = si+tspan/dt;
            if (ei > ((endTime-startTime)/dt).asSeconds())
                ei = ((endTime-startTime)/dt).asSeconds();
            for(uint32_t i = si; i < ei; ++i)
            {
                if(!spectrum.count(i))
                {
                    std::vector<float> buffer;
                    as.Seek(i*dt*1000);
                    while(buffer.size() < n)
                    {
                        std::vector<gz4d::AudioPacket::Ptr> ap = as.Next();
                        float fi = 0.0;
                        if(!ap.empty())
                            while(fi < ap[0]->samples.size())
                            {
                                buffer.push_back(ap[0]->samples[fi]);
                                fi += (ap[0]->sample_rate*dt)/float(n);
                            }
                            else
                                break;
                    }
                    if(buffer.size() >= n)
                    {
                        std::vector<std::complex<float> > s;
                        std::back_insert_iterator<std::vector<std::complex<float> > > ii(s);
                        dft.transform(buffer.begin(),ii,n);
                        spectrum[i].clear();
                        for(std::vector<std::complex<float> >::iterator v = s.begin(); v != s.end(); ++v)
                            spectrum[i].push_back(10.0*log10(abs(*v)));
                    }
                }
                if(gz4d::time::Now() > timeout && !recording)
                    i=ei;
            }
        }
        if(si > 2.0*tspan/dt)
        {
            spectrum.erase(spectrum.begin(),spectrum.lower_bound(si-2.0*tspan/dt));
        }
        spectrum.erase(spectrum.upper_bound(ei+2.0*tspan/dt),spectrum.end());

        globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
    }
}

void SoundDtag::DrawOrtho()
{
    if(ei>si)
    {
        gz4d::glo::PushAttrib attrib;

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        EnableTexture(TEXTURE_COORD_NONE);
        std::shared_ptr<TextureMap> currentTexture;
        if (!customtextureptr)
            currentTexture = *defaulttextureptr;
        else
            currentTexture = customtextureptr;

        float x1 = .1;
        float dx = .8/(tspan/dt);
        float x2 = x1 + dx;

        for(uint32_t i = si; i < ei; ++i)
        {
            if(spectrum.count(i))
            {
                glColor3f(1.0,1.0,1.0);
                glBegin(GL_QUAD_STRIP);
                for(unsigned int j = 0; j < spectrum[i].size()/2; ++j)
                {
                    float y = .025 + .15*j/float(spectrum[i].size()/2);
/*                    float c = 2.5*(100.0+spectrum[i][j])/100.0;
                    glColor3f(c,c,0.1+c);*/
                    currentTexture->SetHeight(spectrum[i][j]);
                    glVertex2f(x1,y);
                    if(spectrum.count(i+1) && j < spectrum[i].size())
                        currentTexture->SetHeight(spectrum[i+1][j]);
                    glVertex2f(x2,y);
                }
                glEnd();
            }
            x1 = x2;
            x2 += dx;
        }
        DisableTexture();

        glColor3f(0.0,0.0,1.0);
        glLineWidth(2.0);
        glBegin(GL_LINES);
        glVertex2f(0.5,0.02);
        glVertex2f(0.5,0.18);
        glEnd();
    }
}

// version called by video recorder
void SoundDtag::fillBuffer(std::vector<int16_t> &buffer, double &t, double timescale)
{
    if(stream && lastTime >= startTime && lastTime <= endTime)
    {
        boost::mutex::scoped_lock lock(mutex);

        if(t == 0.0)
            p.clear();

        if(lastTime-startTime>soundTime+gz4d::seconds(.5) || lastTime-startTime < soundTime-gz4d::seconds(.5))
        {
            stream->Seek((lastTime-startTime).asSeconds()*1000);
        }

        unsigned long i = 0;

        bool done = false;
        while(i < buffer.size()/2 && !done)
        {
            if(p.empty())
            {
                p = stream->Next();
                cursor = 0;
                if(p.empty())
                    done = true;
                else
                {
                    soundTime = (p[0]->timestamp - startTime) + gz4d::seconds(p[0]->samples.size()/double(p[0]->sample_rate));
                    //std::cerr << p[0]->timestamp << std::endl;
                }
            }

            if(!done)
            {
                float rate_ratio = p[0]->sample_rate/44100.0;
                for(unsigned int si = 0; si < buffer.size()/2 - i && cursor+si*rate_ratio+1 < p[0]->samples.size(); ++si)
                {
                    float j = cursor + si*rate_ratio;
                    int ji = (int)j;
                    float jr = j-ji;
                    buffer[(i+si)*2] = 0x7fff*(p[0]->samples[ji]*(1.0-jr) + p[0]->samples[ji+1]*jr);
                    if(p.size() > 1)
                        buffer[(i+si)*2+1] = 0x7fff*(p[1]->samples[ji]*(1.0-jr) + p[1]->samples[ji+1]*jr);
                    else
                        buffer[(i+si)*2+1] = buffer[(i+si)*2];
                }
                if(buffer.size()/2-i < (p[0]->samples.size()-cursor)/rate_ratio)
                {
                    cursor += (buffer.size()/2-i)*rate_ratio;
                    i = buffer.size()/2;
                }
                else
                {
                    i += (p[0]->samples.size()-cursor)/rate_ratio;
                    p.clear();
                }
            }
        }
        t += buffer.size()/(2.0*44100.0);
    }
    else
    {
        memset(&buffer.front(),0,buffer.size()*sizeof(int16_t));
        t += buffer.size()/(2.0*44100.0);
    }
}


// version called by SDL hook
void SoundDtag::fillBuffer(float * data, unsigned long len)
{
    if(playing && stream && !recording)
    {
        boost::mutex::scoped_lock lock(mutex);

        if(lastTime-startTime>soundTime+gz4d::seconds(.5) || lastTime-startTime < soundTime-gz4d::seconds(.5))
        {
            stream->Seek((lastTime-startTime).asSeconds()*1000);
        }

        unsigned long i = 0;
        bool done = false;
        while(i < len && !done)
        {
            if(p.empty())
            {
                p = stream->Next();
                cursor = 0;
                if(p.empty())
                    done = true;
                else
                {
                    soundTime = (p[0]->timestamp - startTime) + gz4d::seconds(p[0]->samples.size()/double(p[0]->sample_rate));
                    //std::cerr << p[0]->timestamp << std::endl;
                }
            }
            if(!done)
            {
                float rate_ratio = p[0]->sample_rate/44100.0;
                for(unsigned int si = 0; si < len - i && cursor+si*rate_ratio+1 < p[0]->samples.size(); ++si)
                {
                    float j = cursor + si*rate_ratio;
                    int ji = (int)j;
                    float jr = j-ji;
                    data[(i+si)*2] = p[0]->samples[ji]*(1.0-jr) + p[0]->samples[ji+1]*jr;
                    if(p.size() > 1)
                        data[(i+si)*2+1] = p[1]->samples[ji]*(1.0-jr) + p[1]->samples[ji+1]*jr;
                    else
                        data[(i+si)*2+1] = data[(i+si)*2];
                }
                if(len-i < (p[0]->samples.size()-cursor)/rate_ratio)
                {
                    cursor += (len-i)*rate_ratio;
                    i = len;
                }
                else
                {
                    i += (p[0]->samples.size()-cursor)/rate_ratio;
                    p.clear();
                }
            }
        }
    }
    else
        memset(data,0,len*2*sizeof(float));
}



