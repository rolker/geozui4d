// c headers
#include<stdio.h>
#include<stdlib.h>

// c++ headers
#include<iostream>

#include"VideoRecorder.h"
#include"MainWindow.h"
#include"engine/Scene3D.h"
#include"engine/TimeControl.h"
#include"objects/Sound.h"
#include <gz4d/glo.h>
#include "utilities/Utilities.h"

extern MainWindow *mainWindow;

VideoRecorder::VideoRecorder()
{

    recordingAudio = false;

    w = 720;
    h = 480;
    fps = 30;
    videoBitrate = 9800000; //9.8 Mbps for dvd
    audioBitrate = 192000;
    startTime = 0.0;
    spatialAA = 1.0;
    watermark = true;
    preview = true;
    recordedFrameFlag = false;
}

VideoRecorder::~VideoRecorder()
{
}

void VideoRecorder::bind()
{
    msosb->Bind(GL_FRAMEBUFFER);
}

void VideoRecorder::unbind()
{
    msosb->Unbind();
}

bool VideoRecorder::Init()
{
    currentFrameTime = 0.0;
    currentSampleTime = 0.0;
    frameno = 0;

    if (watermark)
        mainWindow->ShowWatermark();
    else
        mainWindow->ShowWatermark(false);

    if (preview)
        return true;

    osb = gz4d::glo::Framebuffer::Ptr(new gz4d::glo::Framebuffer(mainWindow->getGloContext()));
    crb = gz4d::glo::Renderbuffer::Ptr(new gz4d::glo::Renderbuffer(mainWindow->getGloContext(),GL_RGBA8));
    //drb = gz4d::glo::Renderbuffer::Ptr(new gz4d::glo::Renderbuffer(GL_DEPTH_STENCIL));

    osb->Bind();
    crb->Bind();
    crb->Storage(w,h);
    osb->Attach(*crb);

    osb->CheckStatus();
    osb->Unbind();

    msosb = gz4d::glo::Framebuffer::Ptr(new gz4d::glo::Framebuffer(mainWindow->getGloContext()));
    mscrb = gz4d::glo::Renderbuffer::Ptr(new gz4d::glo::Renderbuffer(mainWindow->getGloContext(),GL_RGBA8));
    msdrb = gz4d::glo::Renderbuffer::Ptr(new gz4d::glo::Renderbuffer(mainWindow->getGloContext(),GL_DEPTH_STENCIL));

    msosb->Bind();
    mscrb->Bind();
    mscrb->Storage(spatialAA, w,h);
    msosb->Attach(*mscrb);

    msdrb->Bind();
    msdrb->Storage(spatialAA, w,h);
    msosb->Attach(*msdrb);

    msosb->CheckStatus();
    msosb->Unbind();
    std::cerr << "spatial AA: " << spatialAA << std::endl;

//     int max = gz4d::glo::Renderbuffer::GetMaxSize();
// 
//     int t;
//     if(w>h)
//         t = w;
//     else
//         t = h;
// 
//     aa = max/float(t);
//     if (aa > spatialAA)
//         aa = spatialAA;
// 
//     fbw = (unsigned short)(w*aa);
//     fbh = (unsigned short)(h*aa);
// 
//     std::cout << "Requested AA: " << spatialAA << std::endl;
//     std::cout << "obtained AA: " << aa << std::endl;
// 
//     osb->Bind();
//     crb->Bind();
//     crb->Storage(fbw,fbh);
//     osb->Attach(*crb);
// 
//     drb->Bind();
//     drb->Storage(fbw,fbh);
//     osb->Attach(*drb);
// 
//     osb->CheckStatus();
//     osb->Unbind();

   if(Sound::haveCurrentSound())
       recordingAudio = true;

#ifdef GZ4D_AV
    avw.SetVideoBitrate(videoBitrate);
    avw.SetSize(w,h);
    avw.SetFPS(fps);
    avw.SetAudioBitrate(audioBitrate);
    avw.SetAudioSampleRate(44100);
    avw.SetAudioChannels(2);

    avw.Open(filename.string(),recordingAudio);

    audioData.resize(avw.GetAudioFrameSize()*2);
#endif

    data = new unsigned char[h*w*4];
    frame.Resize(w,h); // = new unsigned char[h*w*4];
    multiFrameBuffer = new GLfloat[h*w*4];
    memset(multiFrameBuffer,0,sizeof(GLfloat)*h*w*4);

    Sound::setRecording(1);
    return true;
}

void VideoRecorder::AddFrame(float weight, int startLine, int endLine, bool commit)
{
    if (startLine < 0)
        startLine = 0;
    if (endLine < 0)
        endLine = h;
    //startLine = (unsigned short)(startLine * aa);
    //endLine = (unsigned short) (endLine * aa);

    msosb->Bind(GL_READ_FRAMEBUFFER);
    osb->Bind(GL_DRAW_FRAMEBUFFER);
    glBlitFramebufferEXT(0,0,w,h,0,0,w,h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    osb->Bind(GL_FRAMEBUFFER);
    glReadPixels(0,startLine,w,endLine-startLine,GL_RGBA,GL_UNSIGNED_BYTE,&data[startLine*w*4]);
    osb->Unbind();

    if (commit)
    {
        for(unsigned long i=0;i<h*w*4;i++)
            multiFrameBuffer[i] += float(data[i])*weight/255.0;
    }
}

void VideoRecorder::WriteFrame()
{
    //std::cerr << "frame: " << frameno << " time: " << mainWindow->getScene3D()->timeControl->getGZTime()- startTime << std::endl;

    while(recordingAudio && currentSampleTime < currentFrameTime)
    {
        Sound::fillBufferFromCurrent(audioData,currentSampleTime,mainWindow->getScene3D()->timeControl->getScale());
#ifdef GZ4D_AV
        avw.AddAudio(audioData);
#endif
    }


    gluScaleImage(GL_RGBA,w,h,GL_FLOAT,multiFrameBuffer,w,h,GL_UNSIGNED_BYTE,frame.raw());
    memset(multiFrameBuffer,0,sizeof(GLfloat)*h*w*4);

#ifdef GZ4D_AV
    gz4d::ImageYUV8 yuv(frame.GetRaster());
    avw.AddFrame(yuv);
#endif

    frameno++;
    currentFrameTime += 1.0/fps;
    //currentFrameTime += 1.0/fps*mainWindow->getScene3D()->timeControl->getScale();
    //currentFrameTime = mainWindow->getScene3D()->timeControl->getGZTime();//-startTime;
}

void VideoRecorder::Done()
{
    if (preview)
        return;

#ifdef GZ4D_AV
    avw.Close();
#endif

    Sound::setRecording(0);
    delete[] data;
    data = NULL;
    delete[] multiFrameBuffer;
    multiFrameBuffer = NULL;
    //delete frame;
    //frame = NULL;
}



bool VideoRecorder::setParameter(int argc, const char * argv[])
{
    if (argc > 2)
    {
        std::string parameter(argv[1]);
        if (parameter == "width")
        {
            w = atoi(argv[2]);
            return true;
        }
        else if (parameter == "height")
        {
            h = atoi(argv[2]);
            return true;
        }
        else if (parameter == "fps")
        {
            fps = atoi(argv[2]);
            return true;
        }
        else if (parameter == "audioBitrate")
        {
            audioBitrate = atoi(argv[2]);
            return true;
        }
        else if (parameter == "videoBitrate")
        {
            videoBitrate = atoi(argv[2]);
            return true;
        }
        else if (parameter == "startTime")
        {
            startTime = atof(argv[2]);
            return true;
        }
        else if (parameter == "spatialAA")
        {
            spatialAA = atof(argv[2]);
            return true;
        }
        else if (parameter == "useWatermark")
        {
            watermark = isTrue(std::string(argv[2]));
            return true;
        }
        else if (parameter == "preview")
        {
            preview = isTrue( std::string(argv[2]));
            return true;
        }
        else if (parameter == "filename")
        {
            filename = bfs::path(argv[2]);
            return true;
        }
//         else if (parameter == "codec")
//         {
//             codecID = (CodecID)(atoi(argv[2]));
//             return true;
//         }

    }

    return false;
}

std::string VideoRecorder::getParameter(int argc, const char * argv[])
{
    return "";
}
