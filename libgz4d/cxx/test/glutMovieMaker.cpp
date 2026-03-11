// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/glut/Application.h>
#include <iostream>
#include <gz4d/time.h>
#include <vector>
#include <string>
#include <fstream>
#include <gz4d/socket/TCPSocket.h>
#include <gz4d/util/Utilities.h>
#include <queue>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <gz4d/time/ios.h>

using namespace gz4d::time;

struct BaseFrame
{
    gz4d::Time gzTime;
    double timeScale;
    double x,y,z;
    double h,p,r;
    double scale;
};

struct KeyFrame: public BaseFrame
{
    gz4d::time::Duration frameTime;
};

class GlutMovieMaker: public gz4d::glut::Application<GlutMovieMaker>
{
private:
    int frameCount;
    int midFrame;
    int showFrame;
    int width;
    int height;
    std::map<std::string, std::string> parameters;
    float fps;

    int startx;
    int midFrameStart;
    int selectedFrame;
    bool overviewDrag;
    bool detailDrag;
    bool frameDrag;
    bool deleteFrame;
    bool previewDrag;
    gz4d::time::Duration previewTime;
    bool play;
    gz4d::Time playStart;

    bool record;
    int recordFrame;

    int temporalAAsteps;
    float temporalAAspan;

    std::vector<KeyFrame> frames;
    std::map<gz4d::time::Duration,int> frameMap;
    std::string fname;
    gz4d::TCPSocket gz;
    std::queue<gz4d::time::Duration> SetGZq;

public:
    GlutMovieMaker(std::string const &fname)
    :fname(fname),previewTime(0.0)
    {
        InitWindowSize(1280,100);
        
        
        width = 1280;
        height = 100;
        overviewDrag = false;
        detailDrag = false;
        frameDrag = false;
        deleteFrame = false;
        previewDrag = false;
        play = false;
        record = false;
        temporalAAsteps = 1;
        temporalAAspan = 0.5;

        parameters["filename"] = "out.avi";
        parameters["width"] = "1280";
        parameters["height"] = "720";
        parameters["fps"] = "30";
        fps = 30.0;
        parameters["videoBitrate"] = "9800000";
        parameters["audioBitrate"] = "192000";
        parameters["spatialAA"] = "1.0";

        gz.Connect(gz4d::IPSocket::Address("LOOPBACK",1974));
        std::string ret(1024,0);
        gz.Recv(ret);
        std::cerr << ret << std::endl;
        
        Load(fname);

        if(frames.size() < 2)
        {
            frames.resize(2);
            frames[0].frameTime = gz4d::time::Duration(0.0);
            frames[1].frameTime = gz4d::time::Duration(10.0);
            GetGZ(0);
            GetGZ(1);
            frames[1].h += 90.0;
        }
        FixFrames();
        showFrame = frameCount;
        midFrame = frameCount/2;
    }
    
    void Load(std::string const &fname)
    {
        std::ifstream infile(fname.c_str(),std::ios::binary|std::ios::in);
        if(!infile)
            return;

        std::string backup_fname = fname + "~";
        std::ofstream outfile(backup_fname.c_str(),std::ios::binary|std::ios::out);
        std::vector<char> buffer(512);
        while(!infile.eof())
        {
            infile.read(&buffer.front(),buffer.size());
            outfile.write(&buffer.front(),infile.gcount());
        }

        boost::property_tree::ptree xmldoc;
        read_xml(fname,xmldoc);
        boost::property_tree::ptree::const_assoc_iterator node = xmldoc.find("MovieMakerScript");
        boost::property_tree::ptree::const_assoc_iterator attribs = node->second.find("<xmlattr>");
        for(boost::property_tree::ptree::const_iterator a = attribs->second.begin(); a != attribs->second.end(); ++a)
        {
            parameters[a->first] = a->second.data();
        }
        fps = boost::lexical_cast<float>(parameters["fps"]);
        
        boost::property_tree::ptree::const_assoc_iterator c = node->second.find("TemporalAA");
        if(c != node->second.not_found())
        {
            temporalAAsteps = c->second.get("<xmlattr>.steps",temporalAAsteps);
            temporalAAspan = c->second.get("<xmlattr>.span",temporalAAspan);
        }
        for(boost::property_tree::ptree::const_iterator c = node->second.begin(); c != node->second.end(); ++c)
        {
            if(c->first == "KeyFrame")
            {
                KeyFrame f;
                double t = c->second.get("<xmlattr>.gzTime",0.0);
                f.gzTime = gz4d::Time(t);
                if(c->second.count("<xmlattr>.frameNumber"))
                    f.frameTime = gz4d::time::Duration(c->second.get("<xmlattr>.frameNumber",0)/fps);
                
                f.frameTime = gz4d::time::Duration(c->second.get("<xmlattr>.frameTime",f.frameTime.asSeconds()));
                f.timeScale = c->second.get("<xmlattr>.timeScale",f.timeScale);
                
                f.x = c->second.get("<xmlattr>.x",f.x);
                f.y = c->second.get("<xmlattr>.y",f.y);
                f.z = c->second.get("<xmlattr>.z",f.z);
                
                f.h = c->second.get("<xmlattr>.h",f.h);
                f.p = c->second.get("<xmlattr>.p",f.p);
                f.r = c->second.get("<xmlattr>.r",f.r);
                
                f.scale = c->second.get("<xmlattr>.scale",f.scale);
            }
            
        }

    }
    void Save()
    {
        std::ofstream out(fname.c_str());
        out << "<MovieMakerScript";
        for(std::map<std::string, std::string>::iterator p = parameters.begin(); p != parameters.end(); ++p)
        {
            out << " " << p->first << "=\"" << p->second << "\"";
        }
        out << ">\n";
        out << "  <TemporalAA steps=\"" << temporalAAsteps << "\" span=\"" << temporalAAspan << "\"/>\n";
        for(std::map<gz4d::time::Duration,int>::iterator f = frameMap.begin(); f != frameMap.end(); ++f)
        {
            out << "  <KeyFrame frameTime=\"" << std::fixed << std::setprecision(6) << frames[f->second].frameTime << "\"";
            out << " gzTime=\"" << std::fixed << std::setprecision(6) << frames[f->second].gzTime.secondsSinceEpoch().asSeconds() << "\"";
            out << " timeScale=\"" << frames[f->second].timeScale << "\"";
            out << " x=\"" << frames[f->second].x << "\"";
            out << " y=\"" << frames[f->second].y << "\"";
            out << " z=\"" << frames[f->second].z << "\"";
            out << " h=\"" << frames[f->second].h << "\"";
            out << " p=\"" << frames[f->second].p << "\"";
            out << " r=\"" << frames[f->second].r << "\"";
            out << " scale=\"" << frames[f->second].scale << "\"";
            out << "/>\n";
        }

        out << "</MovieMakerScript>\n";
    }

    void Idle()
    {
        if(record)
        {
            if(recordFrame < frameCount)
            {
                if(temporalAAsteps > 1)
                {
                    for (int i = 0; i < temporalAAsteps; ++i)
                    {
                        previewTime = Duration((recordFrame+(i/(temporalAAsteps-1.0)-1.0)*temporalAAspan)/fps);
                        SetGZ(previewTime);
                        std::stringstream ss;
                        ss << "addFrame " << 1.0/float(temporalAAsteps);
                        SendGZCommand(ss.str());
                    }
                }
                else
                {
                    previewTime = Duration(recordFrame/fps);
                    SetGZ(previewTime);
                    SendGZCommand("addFrame 1.0");
                }
                SendGZCommand("recordFrame");
                ++recordFrame;
            }
            else
            {
                record = false;
                SendGZCommand("stop");
            }

            PostRedisplay();
            return;
        }

        if(play)
        {
            previewTime = gz4d::time::Now() - playStart;
            if(previewTime.asSeconds()*fps > frameCount-1)
            {
                SetGZq.push(Duration((frameCount-1)/fps));
                play = false;
            }
            else
                SetGZq.push(previewTime);
        }
        if(!SetGZq.empty())
        {
            gz4d::time::Duration f;
            bool ok=false;
            while(!SetGZq.empty())
            {
                if(SetGZq.front() >= Duration(0.0) && SetGZq.front() <= Duration(frameCount/fps))
                {
                    f = SetGZq.front();
                    ok = true;
                }
                SetGZq.pop();
            }
            if(ok)
            {
                SetGZ(f);
                PostRedisplay();
            }
        }
    }

    void Draw()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthFunc(GL_ALWAYS);

        glPushMatrix();

        glColor3f(0.5,0.5,0.75);
        for(std::vector<KeyFrame>::iterator f = frames.begin(); f != frames.end(); ++f)
        {
            float x = (f->frameTime.asSeconds()*fps -(midFrame-showFrame/2))/float(showFrame);
            float dx = (height*.95)/(2.0*3.0*width);
            glRectf(x-dx,2.95,x+dx,2.05);
            glBegin(GL_LINES);
            glVertex2f(x,1.0);
            glVertex2f(x,2.95);
            glEnd();
        }
        if(frameDrag)
        {
            if(deleteFrame)
                glColor3f(1,0.3,0.3);
            else
                glColor3f(0.3,0.3,1);
            float x = (frames[selectedFrame].frameTime.asSeconds()*fps -(midFrame-showFrame/2))/float(showFrame);
            float dx = (height*.95)/(2.0*3.0*width);
            glRectf(x-dx,2.95,x+dx,2.05);
            glBegin(GL_LINES);
            glVertex2f(x,1.0);
            glVertex2f(x,2.95);
            glEnd();
        }
        
        glColor3f(0.5,0.0,0.0);
        glBegin(GL_LINES);
        for(int f = midFrame-showFrame/2; f <= midFrame+showFrame/2; ++f)
        {
            if(f >= 0 && f < frameCount)
            {
                glColor3f(0.5,0.5,0.5);
            }
            else
            {
                glColor3f(0.5,0.0,0.0);
            }

            float x = (f-(midFrame-showFrame/2))/float(showFrame);
            glVertex2f(x,1.0);
            glVertex2f(x,1.1);
        }
        for(float s = floor((midFrame-showFrame/2)/fps); s <= (midFrame+showFrame/2)/fps; s += 1.0)
        {
            if(s*fps >= 0.0 && s*fps < frameCount)
            {
                glColor3f(0.5,0.5,0.5);
            }
            else
            {
                glColor3f(0.5,0.0,0.0);
            }

            float h = .25;
            if (fmod(s,5.0f) == 0.0)
                h = .35;
            if (fmod(s,10.0f) == 0.0)
                h = .5;
            if (fmod(s,30.0f) == 0.0)
                h = .75;
            if (fmod(s,60.0f) == 0.0)
                h = 1.0;
            float x = (s*fps-(midFrame-showFrame/2))/float(showFrame);
            glVertex2f(x,1.0);
            glVertex2f(x,1.0+h);
        }
        glEnd();

        glColor3f(0.5,0.5,0.5);
        glRectf((midFrame-showFrame/2)/float(frameCount),0.95,(midFrame+showFrame/2)/float(frameCount),0.05);

        for(float s = ceil((midFrame-showFrame/2)/fps); s <= (midFrame+showFrame/2)/fps; s += 1.0)
        {
            bool print = false;
            float spp = (showFrame/fps)/float(width);
            if(spp < 0.01)
                print = true;
            else if(spp < 0.05 && fmod(s,5.0f) == 0.0)
                print = true;
            else if(spp < 0.1 && fmod(s,10.0f) == 0.0)
                print = true;
            else if(spp < 0.3 && fmod(s,30.0f) == 0.0)
                print = true;
            else if(spp < 0.6 && fmod(s,60.0f) == 0.0)
                print = true;
            if(print)
                PrintTime(Duration(s),1.0+5/float(height));
        }

        if(play||record)
        {
            glColor3f(0.8,0.8,0.8);
            PrintTime(previewTime,1.5);
        }
        else
            glColor3f(0.8,0.3,0.3);
        glBegin(GL_LINES);
        glVertex2f((previewTime.asSeconds()*fps -(midFrame-showFrame/2))/float(showFrame),1.0);
        glVertex2f((previewTime.asSeconds()*fps -(midFrame-showFrame/2))/float(showFrame),3.0);
        glEnd();

        if(record)
        {
            gz4d::time::Duration elapsed = gz4d::time::Now() - playStart;
            if(elapsed > Duration(0.0) && recordFrame > 0)
            {
                gz4d::time::Duration estimated = frameCount*elapsed/double(recordFrame);
                gz4d::time::Duration remaining = estimated - elapsed;
                std::stringstream ss;
                ss << "elapsed: " << GetTimeString(elapsed.asSeconds()) << " estimated: " << GetTimeString(estimated.asSeconds()) << " remaining: " << GetTimeString(remaining.asSeconds());
                glRasterPos2f(0.1,0.1);
                for(int i = 0; i < ss.str().size(); ++i)
                    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, ss.str()[i]);
            }
        }

        if(frameDrag)
            PrintTime(frames[selectedFrame].frameTime,1.5);
        
        if(frameDrag)
        {
            gz4d::time::Duration prevFrameTime(0.0);
            gz4d::Time prevGZTime = frames[frameMap.begin()->second].gzTime;

            gz4d::time::Duration currentFrameTime = frames[selectedFrame].frameTime;
            gz4d::Time currentGZTime = frames[selectedFrame].gzTime;

            for(std::vector<KeyFrame>::iterator f = frames.begin(); f != frames.end(); ++f)
                if(f->frameTime < currentFrameTime && f->frameTime > prevFrameTime)
            {
                prevFrameTime = f->frameTime;
                prevGZTime = f->gzTime;
            }

            if(currentFrameTime != prevFrameTime)
            {
                double tScale = ratio(currentGZTime - prevGZTime,currentFrameTime - prevFrameTime);
                float x = (frames[selectedFrame].frameTime.asSeconds()*fps -(midFrame-showFrame/2))/float(showFrame);
                float dx = (height*.95)/(2.0*3.0*width);
                int sw = 5;
                std::stringstream s;
                s << "Time scale: " << tScale;
                for(int i = 0; i < s.str().size(); ++i)
                    sw += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12,s.str()[i]);
                glRasterPos2f(x-dx-sw/float(width),2.1);
                glColor3f(.75,.75,.75);
                for(int i = 0; i < s.str().size(); ++i)
                    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s.str()[i]);
            }
        }

        if(previewDrag)
            PrintTime(previewTime,1.5);

        glPopMatrix();
        SwapBuffers();
    }

    void Reshape(int w, int h)
    {
        height = h;
        width = w;
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0,1.0,0.0,3.0,-1.0,1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    std::string GetTimeString(double t)
    {
        std::stringstream timeStr;
        if(t < 0.0)
            timeStr << "-";
        t = fabs(t);
        if (t >= 3600.0)
            timeStr <<  int(t/3600) << ":" << std::setfill('0') << std::fixed << std::setw(2) << int(t/60)%60 << ":" << std::setw(6) << std::setprecision(3) << fmod(t,60.0);
        else
            timeStr <<  int(t/60) << ":" << std::setfill('0') << std::fixed << std::setw(6) << std::setprecision(3) << fmod(t,60.0);
        return timeStr.str();
    }

    void PrintTime(gz4d::time::Duration t, float y)
    {
        glColor3f(0.75,0.75,0.75);
        glRasterPos2f((t.asSeconds()*fps -(midFrame-showFrame/2))/float(showFrame),y);

        std::string timeStr = GetTimeString(t.asSeconds());
        for(int i = 0; i < timeStr.size(); ++i)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, timeStr[i]);
    }

    void Mouse(int button, int state, int x, int y)
    {
        
        float yf = 3.0*(height-y)/float(height);
        if(button == 3)
            showFrame = showFrame*.9;
        if(button == 4)
            showFrame = showFrame/.9;
        FixView();
        if (button == GLUT_LEFT_BUTTON)
            if(state == GLUT_DOWN)
            {
                if(yf< 1.0)
                {
                    startx = x;
                    midFrameStart = midFrame;
                    overviewDrag = true;
                }
                else if (yf > 1.0 && yf < 2.0)
                {
                    startx = x;
                    midFrameStart = midFrame;
                    detailDrag = true;
                }
                else if (yf > 2.0 && yf < 3.0 && !(play || record))
                {
                    for (int f = 0; f < frames.size(); ++f)
                    {
                        float fx = (frames[f].frameTime.asSeconds()*fps -(midFrame-showFrame/2))/float(showFrame);
                        float fdx = (height*.95)/(2.0*3.0*width);
                        if (x/float(width) >= (fx-fdx) && x/float(width) <= (fx+fdx))
                        {
                            selectedFrame = f;
                            frameDrag = true;
                            deleteFrame = false;
                            startx = x;
                            midFrameStart = frames[f].frameTime.asSeconds()*fps;
                            previewTime = frames[f].frameTime;
                            SetGZq.push(previewTime);
                            break;
                        }
                    }
                }
            }
            else
            {
                if(frameDrag)
                {
                    if(deleteFrame && frames.size() > 2)
                    {
                        frames.erase(frames.begin()+selectedFrame);
                    }
                    else
                        previewTime = frames[selectedFrame].frameTime;
                    FixFrames();
                    Save();
                }
                overviewDrag = false;
                detailDrag = false;
                frameDrag = false;
            }
        if (button == GLUT_RIGHT_BUTTON  && !(play || record))
            if(state == GLUT_DOWN)
            {
                if(yf< 1.0)
                {
                    previewTime = Duration(std::max(float(0.0),((midFrame-showFrame/2) + x*showFrame/float(width))/fps));
                    playStart = gz4d::time::Now()-previewTime;
                    play = true;
                }
                if (yf > 1.0 && yf < 2.0)
                {
                    previewDrag = true;
                    previewTime = Duration(((midFrame-showFrame/2) + x*showFrame/float(width))/fps);
                    SetGZq.push(previewTime);
                }
                else if (yf > 2.0 && yf < 3.0)
                {
                    int selected = -1;
                    for (int f = 0; f < frames.size(); ++f)
                    {
                        float fx = (frames[f].frameTime.asSeconds()*fps -(midFrame-showFrame/2))/float(showFrame);
                        float fdx = (height*.95)/(2.0*3.0*width);
                        if (x/float(width) >= (fx-fdx) && x/float(width) <= (fx+fdx))
                        {
                            selected = f;
                            break;
                        }
                    }
                    if(selected == -1)
                    {
                        int newFrameNumber = (midFrame-showFrame/2)+x*showFrame/width;
                        bool ok = true;
                        for(std::vector<KeyFrame>::iterator f = frames.begin(); f != frames.end(); ++f)
                        {
                            if(int(f->frameTime.asSeconds()*fps) == newFrameNumber)
                                ok = false;
                        }
                        if(ok)
                        {
                            KeyFrame newFrame;
                            newFrame.frameTime = Duration(newFrameNumber/fps);
                            frames.push_back(newFrame);
                            GetGZ(frames.size()-1);
                            FixFrames();
                            Save();
                        }
                    }
                    else
                    {
                        GetGZ(selected);
                        Save();
                    }
                }
            }
            else
                previewDrag = false;
        PostRedisplay();

    }

    void FixFrames()
    {
        if(frames.size())
        {
            gz4d::time::Duration minFrame = frames[0].frameTime;
            for(std::vector<KeyFrame>::iterator f = frames.begin(); f != frames.end(); ++f)
            {
                if(f->frameTime < minFrame)
                    minFrame = f->frameTime;
            }
            midFrame -= minFrame.asSeconds();
            for(std::vector<KeyFrame>::iterator f = frames.begin(); f != frames.end(); ++f)
            {
                f->frameTime -= minFrame;
            }
            gz4d::time::Duration maxFrame = frames[0].frameTime;
            for(std::vector<KeyFrame>::iterator f = frames.begin(); f != frames.end(); ++f)
            {
                if(f->frameTime > maxFrame)
                    maxFrame = f->frameTime;
            }
            frameCount = ceil(maxFrame.asSeconds()*fps);
            if(frameCount < fps)
                frameCount = fps;
            frameMap.clear();
            for(int f = 0; f < frames.size(); ++f)
                frameMap[frames[f].frameTime] = f;
        }
    }

    void FixView()
    {
        if(showFrame < 10)
            showFrame = 10;
        if(showFrame > 10*frameCount)
            showFrame = 10*frameCount;
        if(midFrame < -showFrame/2)
            midFrame = -showFrame/2;
        if(midFrame > frameCount+showFrame/2)
            midFrame = frameCount+showFrame/2;
    }

    void Motion(int x, int y)
    {
        if(overviewDrag)
        {
            float dx = x-startx;
            midFrame = midFrameStart + dx*frameCount/float(width);
            FixView();
            PostRedisplay();
        }
        if(detailDrag)
        {
            float dx = x-startx;
            midFrame = midFrameStart - dx*showFrame/float(width);
            FixView();
            PostRedisplay();
        }
        if(frameDrag)
        {
            if(y < 0 || y > height)
            {
                deleteFrame = true;
            }
            else
            {
                deleteFrame = false;
                float dx = x-startx;
                int newFrameNumber = midFrameStart + dx*showFrame/float(width);
                bool ok = true;
                for(std::vector<KeyFrame>::iterator f = frames.begin(); f != frames.end(); ++f)
                {
                    if(int(f->frameTime.asSeconds()*fps) == newFrameNumber)
                        ok = false;
                }
                if(ok)
                    frames[selectedFrame].frameTime = Duration(newFrameNumber/fps);
            }
            PostRedisplay();
        }
        if(previewDrag)
        {
            previewTime = Duration(((midFrame-showFrame/2) + x*showFrame/float(width))/fps);
            SetGZq.push(previewTime);
        }
    }

    void GetGZ(int frame)
    {
        gz.Send(std::string("gz posable get Window.main xyzhprS\n"));
        std::string ret(1024,0);
        gz.Recv(ret);
        //std::cerr << ret << std::endl;
        std::stringstream ss(ret);
        int retcode;
        ss >> retcode;
        ss >> frames[frame].x >> frames[frame].y >> frames[frame].z >> frames[frame].h >> frames[frame].p >> frames[frame].r >> frames[frame].scale;
        gz.Send(std::string("gztime get\n"));
        ret.resize(1024);
        gz.Recv(ret);
        ss.str(ret);
        ss >> retcode;
        double t;
        ss >> t;
        frames[frame].gzTime = gz4d::Time(t);
        frames[frame].timeScale = 1.0;
        //std::cerr << ret << std::endl;
    }
    
    void SetGZ(gz4d::time::Duration frame)
    {
        //std::cerr << "SetGZ " << frame << std::endl;
        if(frame >= Duration(0.0) && frame <= Duration((frameCount-1)/fps))
        {
            std::map<gz4d::time::Duration,int>::iterator f1,f2;
            double p,ip;
            f2 = frameMap.lower_bound(frame);
            f1 = f2;
            if(f1 != frameMap.begin())
                --f1;
            if(f1 == f2)
            {
                p = 0.0;
            }
            else
            {
                p = (frame-f1->first).asSeconds()/(f2->first-f1->first).asSeconds();
            }
            ip = 1.0-p;
            //std::cerr << "p: " << p << std::endl;
            BaseFrame iframe;
            iframe.gzTime = interpolate(frames[f1->second].gzTime,frames[f2->second].gzTime, p);
            if(f1 == f2)
                iframe.timeScale = 1.0;
            else
                iframe.timeScale = ratio(frames[f2->second].gzTime-frames[f1->second].gzTime,frames[f2->second].frameTime-frames[f1->second].frameTime);
            iframe.x = frames[f1->second].x*ip+frames[f2->second].x*p;
            iframe.y = frames[f1->second].y*ip+frames[f2->second].y*p;
            iframe.z = frames[f1->second].z*ip+frames[f2->second].z*p;
            //iframe.h = frames[f1->second].h*ip+frames[f2->second].h*p;
            iframe.h = gz4d::InterpolateDegrees(frames[f1->second].h,frames[f2->second].h,p);
            iframe.p = frames[f1->second].p*ip+frames[f2->second].p*p;
            iframe.r = frames[f1->second].r*ip+frames[f2->second].r*p;
            iframe.scale = frames[f1->second].scale*ip+frames[f2->second].scale*p;
            SetGZ(iframe);
        }
    }
    
    void SetGZ(BaseFrame const &f)
    {
        std::stringstream ss;
        ss << "gz posable set Window.main xyzhprS " << std::fixed << std::setprecision(6) <<  f.x << " " << f.y << " " << f.z << " " << f.h << " " << f.p << " " << f.r << " " << f.scale << "; ";
        ss << "gztime set " << std::fixed << std::setprecision(6) << f.gzTime.secondsSinceEpoch().asSeconds() << " " << f.timeScale << "\n";
        //std::cerr << ss.str();
        gz.Send(ss.str());
        std::string ret(1024,0);
        gz.Recv(ret);
    }

    void SendGZCommand(std::string const & cmd)
    {
        std::string c = "global mainWindow; $mainWindow videoRecorder " + cmd + "\n";
        gz.Send(c);
        std::string ret(1024,0);
        gz.Recv(ret);
    }

    void StartRecording()
    {
        SendGZCommand("set preview off");
        for(std::map<std::string, std::string>::iterator p = parameters.begin(); p != parameters.end(); ++p)
        {
            std::stringstream ss;
            ss << "set " << p->first << " " << p->second;
            SendGZCommand(ss.str());
        }

        SendGZCommand("set useWatermark 1");
        std::stringstream ss;
        ss << "set startTime " << std::fixed << frames[frameMap.begin()->second].gzTime.secondsSinceEpoch().asSeconds();
        SendGZCommand(ss.str());
        SendGZCommand("start");
        record = true;
        recordFrame = 0;
        playStart = gz4d::time::Now();
    }

    void Keyboard(unsigned char k, int x, int y)
    {
        std::cerr << int(k) << std::endl;
        switch(k)
        {
            case 27: //escape
                play = false;
                record = false;
                SendGZCommand("stop");
                PostRedisplay();
                break;
            case 'p':
                if(!record && !frameDrag && !previewDrag)
                {
                    play = true;
                    playStart = gz4d::time::Now();
                    previewTime = Duration(0.0);
                }
                break;
            case 'r':
                if(!record && !frameDrag && !previewDrag)
                {
                    play = false;
                    StartRecording();
                }
                break;
            default:
                break;
        }
    }
};


int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        std::cerr << "usage: glutMovieMaker script.xml" << std::endl;
        exit(1);
    }
    GlutMovieMaker mm(argv[1]);
    mm("Movie Maker");
    return 0;
}
