// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gz4d/Image.h>
#include<gz4d/glo/Texture.h>
#include<gz4d/glut/Application.h>
#include <gz4d/av.h>
#include <iostream>
#include <gz4d/time/ios.h>
#include <fstream>
#include <boost/filesystem.hpp>

class MyScene: public gz4d::glut::Application<MyScene,gz4d::glut::Ortho<> >
{
    private:
        gz4d::ImageRGBA8 image;
        gz4d::glo::Texture texture;

        std::shared_ptr<gz4d::AVReader> avr;
        gz4d::AVReader::VideoStream::output_queue_type vq;
        gz4d::AVReader::VideoStream::Ptr vs;
        gz4d::AVReader::VideoFrame currentFrame;

        gz4d::Time videoStart;
        gz4d::time::Duration videoPosition;
        gz4d::time::Duration videoLength;


        int speed;
        float speedFactor;
        bool paused;

        int mouse_start;
        int mouse_last;
        bool seeking;
        gz4d::time::Duration seek_start;

        struct TrialData
        {
            std::string subject;
            std::string run;
            std::string c1;
            std::string c2;
            std::string time;
            std::string timestamp;
        };

        std::vector<TrialData> trials;
        int currentTrial;

        std::vector<std::string> _dataDirs;
        float _heightFactor;
        
        typedef std::map<gz4d::time::Duration,std::string> SectionMap;
        std::vector<SectionMap> sections;

    public:
        MyScene(std::string const &results_filename, int start, std::vector<std::string> const &dataDirs)
        :texture(context,GL_TEXTURE_2D,GL_RGBA),seeking(false),speed(4),speedFactor(0.25*speed),paused(false),currentTrial(-1),_dataDirs(dataDirs),_heightFactor(1.0+.1*(dataDirs.size()+1))
        {
            std::ifstream results(results_filename.c_str());
            while(results && !results.eof())
            {
                TrialData trial;
                std::string d;
                results >> trial.subject >> trial.run >> trial.c1 >> trial.c2 >>  d >> d >> d >> trial.time >> d >> d >> trial.timestamp;
                for(int i = 0; i < trial.timestamp.size(); ++i)
                    if(trial.timestamp[i] == ':')
                        trial.timestamp[i] = '.';
                if(results)
                    trials.push_back(trial);
            }

            if(trials.empty())
            {
                std::cerr << "Unable to load trials" << std::endl;
                exit(1);
            }

            if(start < 0)
                start = 0;
            if(start >= trials.size())
                start = trials.size()-1;

            SelectTrial(start);
            InitWindowSize(2*vs->GetWidth(),2*vs->GetHeight()*_heightFactor);
        }

        void SaveData()
        {
            //CleanSections();
            if (currentTrial >= 0 && currentTrial < trials.size())
            {
                boost::filesystem::path dataFile = _dataDirs[0];
                dataFile /= trials[currentTrial].timestamp+"_"+trials[currentTrial].subject+"_compressed.avi.csv";
                std::ofstream dataOut(dataFile.c_str());
                for(SectionMap::iterator i = sections[0].begin(); i != sections[0].end(); ++i)
                    dataOut << i->first.asSeconds() << "\t" << i->second << std::endl;
            }
        }

//         void CleanSections()
//         {
//             std::string lastSection = "unknown";
//             std::vector<gz4d::time::Duration> trash;
//             for(SectionMap::const_iterator i = sections.begin(); i != sections.end(); ++i)
//                 if(i->second == lastSection)
//                     trash.push_back(i->first);
//                 else
//                     lastSection = i->second;
//             for(std::vector<gz4d::time::Duration>::const_iterator i = trash.begin(); i != trash.end(); ++i)
//                 sections.erase(*i);
//         }

        void SelectTrial(int t)
        {
            sections.clear();

            currentTrial = t;

            for (std::vector<std::string>::const_iterator dd = _dataDirs.begin(); dd != _dataDirs.end(); ++dd)
            {
                sections.push_back(SectionMap());
                boost::filesystem::path dataFile = *dd;
                dataFile /= trials[currentTrial].timestamp+"_"+trials[currentTrial].subject+"_compressed.avi.csv";
                std::ifstream dataIn(dataFile.c_str());
                while(dataIn && !dataIn.eof())
                {
                    double t;
                    std::string target;
                    dataIn >> t >> target;
                    if(dataIn)
                        sections.back()[gz4d::seconds(t)] = target;
                }
            }

            std::cerr << currentTrial+1 << " of " << trials.size() << "\t" << trials[currentTrial].run << "\t" << trials[currentTrial].c1 << "\t" << trials[currentTrial].c2 << "\t" << trials[currentTrial].time <<  std::endl;

            std::string vf = trials[currentTrial].timestamp+"_"+trials[currentTrial].subject+"_compressed.avi";
            std::cerr << vf << std::endl;

            avr = std::shared_ptr<gz4d::AVReader>(new gz4d::AVReader);
            if(!avr->Open(vf))
            {
                std::cerr << "failed to open: " << vf << std::endl;
                vs.reset();
                vq.reset();
            }
            else
            {
                vs = avr->GetVideoStreams()[0];
                videoLength = vs->GetDuration();
                std::cerr << videoLength.asSeconds() << std::endl;
                vq = vs->NewOutQueue();
                while(vq->empty())
                    avr->ReadFrame();
                currentFrame = vq->pop();
                image = gz4d::ImageRGBA8(currentFrame.frame.GetRaster());
                videoStart = gz4d::time::Now();
                videoPosition = gz4d::seconds(0.0);
            }
        }

        void Draw()
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            if(!texture)
            {
                texture.Bind();
                texture.Image(image.GetRaster());
            }
            else
            {
                texture.Bind();
                image = gz4d::ImageRGBA8(currentFrame.frame.GetRaster());
                texture.SubImage(image.GetRaster());
            }

            glDisable(GL_LIGHTING);

            texture.MinFilter(GL_LINEAR);
            texture.MagFilter(GL_NEAREST);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0.0);
            
            int w = image.GetRaster().GetColumns();
            int h = image.GetRaster().GetRows();
            float wf = 1.0;
            float hf = 1.0;
            if (w<h)
                wf = w/float(h);
            if (h<w)
                hf = h/float(w);
            float scale = _canvas_width/wf;
            if(_canvas_height/(hf*_heightFactor) < scale)
                scale = _canvas_height/(hf*_heightFactor);

            wf *= scale;
            hf *= scale;

            float y0 = hf*_heightFactor;
            float y1 = hf*(_heightFactor-2.0);


            glColor3f(1.0,1.0,1.0);
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0,0.0);
            glVertex2f(-wf,y0);
            glTexCoord2f(0.0,1.0);
            glVertex2f(-wf,y1);
            
            glTexCoord2f(1.0,1.0);
            glVertex2f(wf,y1);
            
            glTexCoord2f(1.0,0.0);
            glVertex2f(wf,y0);
            
            glEnd();
            glDisable(GL_TEXTURE_2D);


            float p = 2.0*(1.0-((videoLength- currentFrame.presentationTime).asSeconds()/videoLength.asSeconds()));

            y0 = y1;
            y1 -= hf*0.2;

            glColor3f(0.5,0.5,0.5);
            glBegin(GL_QUADS);
            glVertex2f(-wf,y0);
            glVertex2f(-wf,y1);
            glVertex2f(-wf+p*wf,y1);
            glVertex2f(-wf+p*wf,y0);
            glEnd();

            for(std::vector<SectionMap>::const_iterator s = sections.begin(); s != sections.end(); ++s)
            {
                y0 = y1;
                y1 -= hf*0.2;
                
                glColor3f(1.0,0.0,0.0);
                glBegin(GL_QUADS);
                glVertex2f(-wf,y0);
                glVertex2f(-wf,y1);
                
                for(SectionMap::const_iterator i = s->begin(); i != s->end(); ++i)
                {
                    float p = 2.0*(1.0-((videoLength- i->first).asSeconds()/videoLength.asSeconds()));
                    glVertex2f(-wf+p*wf,y1);
                    glVertex2f(-wf+p*wf,y0);
                    if(i->second == "screen")
                        glColor3f(0.0,1.0,0.0);
                    else if(i->second == "outside")
                        glColor3f(0.0,0.0,1.0);
                    else
                        glColor3f(1.0,0.0,0.0);
                    glVertex2f(-wf+p*wf,y0);
                    glVertex2f(-wf+p*wf,y1);
                }
                glVertex2f(wf,y1);
                glVertex2f(wf,y0);
                glEnd();

                glColor3f(0.0,0.0,0.0);
                glBegin(GL_LINES);
                for(SectionMap::const_iterator i = s->begin(); i != s->end(); ++i)
                {
                    float p = 2.0*(1.0-((videoLength- i->first).asSeconds()/videoLength.asSeconds()));
                    glVertex2f(-wf+p*wf,y1);
                    glVertex2f(-wf+p*wf,y0);
                }
                glEnd();
            }
            
            SwapBuffers();
        }

        void Idle()
        {

            gz4d::time::Duration lastCurrentTime = currentFrame.presentationTime;

            gz4d::Time now = gz4d::time::Now();
            if(!seeking && !paused)
            {
                videoPosition = (now - videoStart)*speedFactor;

                while (videoPosition >= videoLength)
                    videoPosition -= videoLength;
                while (videoPosition < gz4d::seconds(0.0))
                    videoPosition += videoLength;
            }
            videoStart = now-videoPosition/speedFactor;

            if(videoPosition < currentFrame.presentationTime || videoPosition > currentFrame.presentationTime+currentFrame.duration)
            {
                if(videoPosition > currentFrame.presentationTime+currentFrame.duration && videoPosition < currentFrame.presentationTime+(currentFrame.duration*2.0))
                    avr->ReadFrame();
                else
                {
                    avr->Seek(videoPosition);
                    //vs->Seek(videoPosition);
                    //avr->ReadFrame();
                }
                if(!vq->empty())
                    currentFrame = vq->pop();
            }

            if(lastCurrentTime != currentFrame.presentationTime)
            {
                    PostRedisplay();
            }
        }



        void Mouse(int button, int state,int x, int y)
        {
            if(button == 0)
            {
                if(state == 0)
                {
                    mouse_start = x;
                    mouse_last = x;
                    seek_start = videoPosition;
                    seeking = true;
                }
                else
                {
                    seeking = false;
                }
            }
            if(button == 3 && state == 0)
            {
                if (paused)
                {
                    paused = false;
                }
                else if (speed < 8)
                {
                    float oldSpeedFactor = speedFactor;
                    ++speed;
                    speedFactor = 0.25*speed;
                    videoStart += videoPosition*(speedFactor-oldSpeedFactor)/(speedFactor*oldSpeedFactor);
                    std::cerr << "speed factor: " << speedFactor << std::endl;
                }
            }
            if(button == 4 && state == 0)
            {
                if (speed > 1)
                {
                    float oldSpeedFactor = speedFactor;
                    --speed;
                    speedFactor = 0.25*speed;
                    videoStart += videoPosition*(speedFactor-oldSpeedFactor)/(speedFactor*oldSpeedFactor);
                    std::cerr << "speed factor: " << speedFactor << std::endl;
                }
                else
                    paused = true;
            }

        }

        void Motion(int x, int y)
        {
            if(seeking)
            {
                if (x != mouse_last)
                {
                    videoPosition = seek_start + videoLength*((x-mouse_start)/float(_window_width));
                    if(videoPosition.asSeconds() < 0.0)
                        videoPosition = gz4d::seconds(0.0);
                    if(videoPosition > videoLength)
                        videoPosition = videoLength;
                    //std::cerr << "seek to: " << seek_to << std::endl;

                    mouse_last = x;
                    PostRedisplay();
                }
            }
        }

        void Keyboard(unsigned char key, int x, int y)
        {
            switch(key)
            {
                case '1':
                    sections[0][currentFrame.presentationTime] = "screen";
                    SaveData();
                    PostRedisplay();
                    break;
                case '2':
                    sections[0][currentFrame.presentationTime] = "outside";
                    SaveData();
                    PostRedisplay();
                    break;
                case '3':
                    sections[0][currentFrame.presentationTime] = "unknown";
                    SaveData();
                    PostRedisplay();
                    break;
                case 'c':
                    if(!sections[0].empty() && sections[0].begin()->first <= videoPosition)
                    {
                        //std::cerr << "position: " << videoPosition << std::endl;
                        SectionMap::const_iterator clearItem = sections[0].upper_bound(currentFrame.presentationTime);
                        //if(clearItem != sections.end())
                            //std::cerr << "clearItem: " << clearItem->first << std::endl;
                        --clearItem;
                        sections[0].erase(clearItem->first);
                        SaveData();
                    }
                    PostRedisplay();
                    break;
                case ' ':
                    paused = !paused;
                    break;
                default:
                    break;
                    //std::cerr << "keyboard: (" << int(key) << ") " << key << std::endl;
            }
        }

        void Special(int key, int x, int y)
        {
            
            switch(key)
            {
                case GLUT_KEY_PAGE_UP:
                    if(currentTrial > 0)
                        SelectTrial(currentTrial-1);
                    PostRedisplay();
                    break;
                case GLUT_KEY_PAGE_DOWN:
                    if(currentTrial < trials.size()-1)
                        SelectTrial(currentTrial+1);
                    PostRedisplay();
                    break;
                default:
                    std::cerr << "special: (" << key << ") unknown" << std::endl;;
            }

        }

};

int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        std::cerr << "usage: videoAnalyzer results.csv [trialNo  [datadir1 datadir2 ...]]\n\tTrial number defaults to 31\n\tData dir is . is none specified" << std::endl;
        return 1;
    }

    int start = 30;
    if (argc >= 3)
        start = atoi(argv[2])-1;
    
    std::vector<std::string> dataDirs;
    if (argc <= 3)
        dataDirs.push_back("./");
    else
    {
        for(int i = 3; i < argc; ++i)
            dataDirs.push_back(argv[i]);
    }

    MyScene s(argv[1],start,dataDirs);
    s("Video Analyzer");
    return 0;
}
