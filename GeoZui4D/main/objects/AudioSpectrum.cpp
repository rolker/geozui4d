// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "AudioSpectrum.h"
#include "engine/Scene3D.h"
#include "engine/Window3D.h"
#include "engine/InputMediator.h"
#include <gz4d/Color.h>

REGISTER_OBJECT(AudioSpectrum,"");

AudioSpectrum::AudioSpectrum()
:N(2048),paDevice(paContext.getDefaultInputDevice()),stream(paContext.getDefaultFormat().open())
    //:dft(2048)
{
    history = 16;
    fft_overlap = 4;
    tindex = 0;
    findex = 0;
    //OpenPulseAudio(16384);
    OpenJack();
}

void AudioSpectrum::OpenPulseAudio(uint32_t sampleRate)
{
    //gz4d::AudioDevice::Stream::DefaultInputParameters p;
    //gz4d::AudioDevice::Stream::Format f(p,gz4d::AudioDevice::Stream::Parameters(),sampleRate);
    //stream.Open(f,&paDevice);
    //fs = stream.GetSampleRate();
    Open();
    dataQueue = stream.NewOutQueue();
    stream.start();
}

void AudioSpectrum::OpenJack()
{
#ifndef WIN32
    jack = gz4d::jack::Client::Ptr(new gz4d::jack::Client("gzSpectrum"));
    gz4d::jack::InputPort::Ptr in = jack->NewInputPort("in");
    fs = jack->GetSampleRate();
    Open();
    dataQueue = in->NewOutQueue();
    jack->Activate();
#endif
}

void AudioSpectrum::Open()
{
    timeDomain.resize(history*fs);
    raster.Resize(N/2,history*fs/(N/fft_overlap));
    avgRaster.Resize(N/2,history*fs/(N/fft_overlap));
}

void AudioSpectrum::Idle(double time)
{
    while(!dataQueue->empty())
    {
        gz4d::AudioPacket::Ptr data = dataQueue->pop();
        for (unsigned long i = 0; i<data->samples.size(); ++i)
        {
            timeDomain[tindex]=data->samples[i];
            tindex++;
            if(tindex >= timeDomain.size())
                tindex = 0;
        }
    }

    bool done = false;
    while (!done)
    {
        gz4d::Pixel<gz4d::RGBA8> *row = NULL;
        gz4d::Pixel<gz4d::RGBA8> *avgRow = NULL;
        std::vector<double> input;
        {
            boost::mutex::scoped_lock lock(tdMutex);
            if (findex < tindex)
            {
                if(findex + N <= tindex)
                {
                    input.insert(input.end(),&timeDomain[findex],&timeDomain[findex+N]);
                    //int r = findex/(n/fft_overlap);
                    //std::cerr << r << ", " << findex << ", " << tindex << ", " << n << ", " << fft_overlap << ", " << n/fft_overlap << std::endl;
                    row = raster.GetRow(findex/(N/fft_overlap));
                    avgRow = avgRaster.GetRow(findex/(N/fft_overlap));
                    findex += N/fft_overlap;
                }
                else
                    done = true;
            }
            else
            {
                if(findex + N < timeDomain.size())
                {
                    input.insert(input.end(),timeDomain.begin()+findex,timeDomain.begin()+findex+N);
                    //int r = findex/(n/fft_overlap);
                    //std::cerr << r << ", " << findex << ", " << tindex << ", " << n << ", " << fft_overlap << ", " << n/fft_overlap << std::endl;
                    row = raster.GetRow(findex/(N/fft_overlap));
                    avgRow = avgRaster.GetRow(findex/(N/fft_overlap));
                    findex += N/fft_overlap;
                }
                else if (findex < timeDomain.size())
                {
                    if(findex + N-tindex < timeDomain.size())
                    {
                        input.insert(input.end(),timeDomain.begin()+findex,timeDomain.end());
                        input.insert(input.end(),&timeDomain[0],&timeDomain[N-(timeDomain.size()-findex)]);
                        row = raster.GetRow(findex/(N/fft_overlap));
                        avgRow = avgRaster.GetRow(findex/(N/fft_overlap));
                        findex += N/fft_overlap;
                    }
                    else
                        done = true;
                }
                else
                    findex = 0;
            }
        }
        if(row)
        {
            frequencyDomain.clear();
            std::back_insert_iterator<std::vector<std::complex<double> > > ii(frequencyDomain);
            dft.transform(input.begin(),ii,N);
            if (frequencyDomainAverages.size() != frequencyDomain.size())
                frequencyDomainAverages.resize(frequencyDomain.size());
            for(unsigned int c = 0; c < frequencyDomain.size() && c < raster.GetColumns(); ++c)
            {
                double db = 20*log10(abs(frequencyDomain[c]));
                float r = (125.0+db)/25.0;
                float g = fabs((2.0*r)-1.0);
                float b = (150.0+db)/75.0;//fabs(1.0-r);

                gz4d::Color color;
                color[0] = r;
                color[1] = g;
                color[2] = b;
                color[3] = 1.0;
                row[c] = color;

                float diff = db - frequencyDomainAverages[c];
                frequencyDomainAverages[c] *= 0.95;
                frequencyDomainAverages[c] += 0.05*db;
                r = diff/9.0;
                g = fabs(diff/12.0);
                b = -diff/9.0;
                color[0] = r;
                color[1] = g;
                color[2] = b;
                avgRow[c] = color;

/*                row[c][0] = db-255;
                row[c][1] = int(50.0*db)%256;
                row[c][2] = fabs(db);*/
//                 if (db > -50)
//                 {
//                     row[c][0] = 5*(db+50);
//                     row[c][1] = 0.0;
//                     row[c][2] = 0.0;
//                 }
//                 if (db > -100 && db <= -50)
//                 {
//                     row[c][0] = 0.0;
//                     row[c][1] = 5*(db+100);
//                     row[c][2] = 0.0;
//                 }
//                 if (db <= -100)
//                 {
//                     row[c][0] = 0.0;
//                     row[c][1] = 0.0;
//                     row[c][2] = (db+355);
//                 }

                //row[c][3] = 255;
            }
        }
    }
    globalInputMediator->Trigger(Scene3D::sceneChangedEvent());

}


void AudioSpectrum::DrawLowRes(float degradation, Window3D * win)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDepthFunc(GL_LEQUAL);

    glDisable(GL_LIGHTING);
    glColor4f(1.0,1.0,1.0,1.0);

    glEnable(GL_TEXTURE_2D);


    if(!texture)
    {
        texture = std::shared_ptr<gz4d::glo::Texture>(new gz4d::glo::Texture(win->getGloContext()));
        texture->Bind();
        texture->MagFilter(GL_NEAREST);
        texture->MinFilter(GL_NEAREST);
        texture->Image(raster);
    }
    else
    {
        texture->Bind();
        texture->SubImage(raster);
    }

    glBegin(GL_QUADS);
    float tr = tindex/float(timeDomain.size());
    glTexCoord2f(1.0,tr);
    glVertex3f(-history,0,fs/2000.0);
    glTexCoord2f(0.0,tr);
    glVertex3f(-history,0,0);
    glTexCoord2f(0.0,tr+1.0);
    glVertex3f(0,0,0);
    glTexCoord2f(1.0,tr+1.0);
    glVertex3f(0,0,fs/2000.0);
    glEnd();

    texture->SubImage(avgRaster);
    glBegin(GL_QUADS);
    //float tr = tindex/float(timeDomain.size());
    glTexCoord2f(1.0,tr);
    glVertex3f(-history,0,10+fs/2000.0);
    glTexCoord2f(0.0,tr);
    glVertex3f(-history,0,10);
    glTexCoord2f(0.0,tr+1.0);
    glVertex3f(0,0,10);
    glTexCoord2f(1.0,tr+1.0);
    glVertex3f(0,0,10+fs/2000.0);
    glEnd();

    texture->Unbind();
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_LINE_STRIP);
    for(unsigned long i = 0; i < frequencyDomain.size(); i++)
    {
        double f = i*(fs/(double)(N));
        glVertex3f(f/1000.0,0.0,20.0/1000.0*log10(abs(frequencyDomain[i])));
    }
    glEnd();

    glBegin(GL_LINE_STRIP);

    boost::mutex::scoped_lock lock(tdMutex);
    for(unsigned long i = 0; i < timeDomain.size(); i++)
    {
        glColor4f(timeDomain[(i + tindex + 1)%timeDomain.size()],0.5,0.3,1.0);
        glVertex3f(-history+i/float(fs),0.0,timeDomain[(i + tindex + 1)%timeDomain.size()]-1.0);

    }
    glEnd();
    glPopAttrib();
}


