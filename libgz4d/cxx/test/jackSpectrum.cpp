// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sound/Jack.h>
#include <gz4d/glut/Application.h>
#include <gz4d/dsp/FastFourierTransform.h>
#include <gz4d/util/SignalGenerator.h>
#include <gz4d/dsp/CrossCorrelation.h>

class JackSpectrum: public gz4d::glut::Application<JackSpectrum>
{
    std::shared_ptr<gz4d::jack::Client> client;
    int bufferSize;
    int samplesPerFrame;

    std::vector<gz4d::jack::InputPort::Ptr> ports;
    std::vector<gz4d::ThreadedQueue<gz4d::AudioPacket::Ptr>::Ptr> port_qs;
    struct Packet
    {
        gz4d::AudioPacket::Ptr audio;
        std::shared_ptr<std::vector<double> > spectrum;
        float crossCorrelation;
    };

    std::vector<std::list<Packet> > audio_packets;

    typedef gz4d::FastFourierTransform<double,256> FFT;

    int minBin, maxBin;
    std::vector<float> signal;

    public:
        
        JackSpectrum(int bufferSize)
            :client(new gz4d::jack::Client("Spectrum1")),bufferSize(bufferSize)
        {
            ports.push_back(client->NewInputPort("in1"));
            ports.push_back(client->NewInputPort("in2"));
            port_qs.push_back(ports[0]->NewOutQueue());
            port_qs.push_back(ports[1]->NewOutQueue());
            samplesPerFrame = client->GetBufferSize();
            std::cerr << "Samples per frame: " << samplesPerFrame << std::endl;
            audio_packets.resize(2);
            
            int sampleRate = client->GetSampleRate();
            float freqStep = sampleRate/float(samplesPerFrame);
            float frequency = 2500.0;
            minBin = (frequency*0.8)/freqStep;
            maxBin = (frequency*1.2)/freqStep;

            gz4d::SignalGenerator sg;
            sg.SetFrequency(frequency);
            sg.SetSamplingFrequency(sampleRate);
            signal.resize(0.01*sampleRate);
            sg.SetTimeLength(0.01);
            sg.FillBuffer(&signal.front(),signal.size());
            client->Activate();
        }

        void Idle()
        {
            for(int i = 0; i < 2; ++i)
            {
                while(!port_qs[i]->empty())
                {
                    Packet p;
                    p.audio = port_qs[i]->pop();
                    p.spectrum = std::shared_ptr<std::vector<double> >(new std::vector<double>);
                    std::vector<float> cc = gz4d::CrossCorrelation<float>(p.audio->samples,signal);
                    p.crossCorrelation = *std::max_element(cc.begin(),cc.end());
                    //dft.Transform(p.audio->samples,*p.spectrum,true);
                    audio_packets[i].push_back(p);
                    if(audio_packets[i].size() > bufferSize)
                        audio_packets[i].pop_front();
                    PostRedisplay();
                }
            }
        }

        void Draw()
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawBuffer(GL_BACK);

            for(int i = 0 ; i < 2; ++i)
            {
                glPushMatrix();
    
/*                glBegin(GL_QUADS);
                float j = -1.0;
                for(std::list<Packet>::iterator ap = audio_packets[i].begin(); ap != audio_packets[i].end(); ++ap)
                {
                    for(int k = minBin; k <= maxBin && k < ap->spectrum->size()/2; ++k)
                    {
                        float y1 = (k-minBin)/float(maxBin-minBin);
                        y1 /= 2.0;
                        float y2 = y1 + 0.5/float(maxBin-minBin);
                        float p = (*ap->spectrum)[k];
                        float c = p*45.0;
                        glColor3f(c,c,c);
                        glVertex3f(j,y2-i,0.0);
                        glVertex3f(j,y1-i,0.0);
                        glVertex3f(j+1.0/float(bufferSize),y1-i,0.0);
                        glVertex3f(j+1.0/float(bufferSize),y2-i,0.0);
                    }
                    j += 1.0/float(bufferSize);
                }
                glEnd();*/

                glBegin(GL_LINE_STRIP);
                float j = -1.0;
                for(std::list<Packet>::iterator ap = audio_packets[i].begin(); ap != audio_packets[i].end(); ++ap)
                {
                    float c = 0.5+ap->crossCorrelation*0.01;
                    glColor3f(c,c,c);
                    glVertex3f(j,ap->crossCorrelation*0.01 +.5-i,0.0);
                    j += 1.0/float(bufferSize);
                }
                glEnd();
                glPopMatrix();
            }
            SwapBuffers();
        }

        void Reshape(int w, int h)
        {
            glViewport(0, 0, w, h);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-1, 0,-1,1,-1.0,1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        void Mouse(int button, int state, int x, int y)
        {
            std::cerr << button << std::endl;
        }

        void Motion(int x, int y)
        {
        }

        void Keyboard(unsigned char key, int x, int y)
        {
            if(key == 'q')
            {
                std::cerr << "Exiting..." << std::endl;
                client.reset();
                Sleep(gz4d::seconds(5));
                std::cerr << "Done" << std::endl;
                exit(0);
            }
        }
};


int main(int argc, char * argv[])
{
    int bufferSize = 25;
    if (argc > 1)
    {
        bufferSize = atoi(argv[1]);
    }

    std::cerr << "Buffer size: " << bufferSize << std::endl;

    JackSpectrum js(bufferSize);
    js("Spectrum");
}
