#ifndef GZ4D_AUDIOSPECTRUM_H
#define GZ4D_AUDIOSPECTRUM_H

#include <gz4d/gz4d.h>
#include <gz4d/dsp/FastFourierTransform.h>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <gz4d/time.h>
#include <gz4d/sound/AudioPacket.h>

namespace gz4d
{
    class AudioSpectrum
    {
        
        typedef gz4d::FastFourierTransform<double,512> FFT;
        
        
        public:
            typedef std::shared_ptr<AudioSpectrum> Ptr;

            struct Bin
            {
                double center;
                double start;
                double end;
                double value;
            };

            AudioSpectrum()
            {
                //frequencyDomain.resize(n/2+1);
                scale = 1.0;
                fs = 0;
                SetBins(1000,20000,100);
            }
            
            void Draw()
            {
                bool flag = false;
                {
                    boost::mutex::scoped_lock lock(tdMutex);
                    if (timeDomain.size() >= FFT::size)
                    {
                        //memmove(&timeDomain.front(),&timeDomain[timeDomain.size()-n],n*sizeof(double));
                        //timeDomain.resize(n);
                        //Transform(timeDomain,frequencyDomain,true);

                        FFT::transform(timeDomain.begin(),std::back_insert_iterator<std::vector<std::complex<double> > >(frequencyDomain));
                        
                        std::vector<double> ntd(timeDomain.begin()+FFT::size,timeDomain.end());
                        timeDomain.swap(ntd);
                        
                        flag = true;
                    }
                }
                
                if(flag)
                {
                    for(std::vector<Bin>::iterator i = bins.begin(); i!=bins.end();i++)
                        i->value = 0.0;
                    std::vector<Bin>::iterator b = bins.begin();
                    for(unsigned long i = 0; i < frequencyDomain.size(); i++)
                    {
                        double f = i*(fs/(double)FFT::size);
                        while(b != bins.end() && b->end < f)
                            b++;
                        if (b != bins.end() && f <= b->end && f>b->start)
                            b->value += norm(frequencyDomain[i]);
                    }
                }
                
                /*    glColor3f(0.8,0.8,1.0);
                 *    glBegin(GL_LINE_STRIP);
                 *    for (int i = 0; i<bins.size();i++)
                 *    {
                 *        glVertex2f((float)i/(float)bins.size(),bins[i].value*scale/(bins[i].end-bins[i].start));
                 }
                 glEnd();*/
            }
            
            void DrawWaveform(int count, bool trigger = false)
            {
                int offset = 0;
                if (trigger)
                {
                    boost::mutex::scoped_lock lock(tdMutex);
                    while(offset < timeDomain.size()-count-1 && !(timeDomain[offset]<0 && timeDomain[offset+1]>=0))
                        offset++;
                }
                double rms = GetRMS();
                /*    glColor3f(0.2,0.6,0.2);
                 * 
                 *    glBegin(GL_QUADS);
                 *    glVertex2f(0.0,.75+rms/4.0);
                 *    glVertex2f(0.0,.75-rms/4.0);
                 *    glVertex2f(1.0,.75-rms/4.0);
                 *    glVertex2f(1.0,.75+rms/4.0);
                 *    glEnd();
                 * 
                 *    glColor3f(0.6,0.3,0.3);
                 *    glBegin(GL_LINE_STRIP);
                 *    {
                 *        boost::mutex::scoped_lock lock(tdMutex);
                 *        for (int i = 0; i<timeDomain.size()&&i<count; i++)
                 *        {
                 *            glVertex2f((float)i/(float)count,.75+timeDomain[i]/4.0);
                 }
                 }
                 glEnd();*/
            }
            
            void SetBins(double start, double end, double interval, bool octave=false)
            {
                bins.clear();
                Bin b;
                b.value = 0.0;
                b.center = start;
                while(b.center <= end)
                {
                    if(octave)
                    {
                        b.start = b.center/sqrt(pow(2.0,interval));
                        b.end = b.start*pow(2.0,interval);
                    }
                    else
                    {
                        b.start = b.center-interval/2.0;
                        b.end = b.start+interval;
                    }
                    bins.push_back(b);
                    if(octave)
                        b.center = b.end*sqrt(pow(2.0,interval));
                    else
                        b.center += interval;
                }
            }
            
            Bin const &operator[](unsigned int i)const{return bins[i];}
            void SetScale(float s){scale = s;}
            unsigned int GetBinCount(){return bins.size();}
            
            double GetRMS()
            {
                boost::mutex::scoped_lock lock(tdMutex);
                if(!timeDomain.empty())
                {
                    double sum = 0.0;
                    for (int i = 0; i < timeDomain.size(); i++)
                        sum += timeDomain[i]*timeDomain[i];
                    return sqrt(sum/(double)timeDomain.size());
                }
                return 0.0;
            }
            
        
            void Consume(AudioPacket::Ptr p)
            {
                boost::mutex::scoped_lock lock(tdMutex);
                timeDomain.insert(timeDomain.end(),p->samples.begin(),p->samples.end());
                fs = p->sample_rate;
            }
    private:
            
            std::vector<std::complex<double> > frequencyDomain;
            std::vector<double> timeDomain;
            float scale;
            double fs;
            std::vector<Bin> bins;
            boost::mutex tdMutex;
    };
}

#endif

