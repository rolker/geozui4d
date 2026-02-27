#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include <string>

// #ifdef WIN32
// #include <AtlImage.h> 
// #endif

#include <gz4d/glo.h>
#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#ifdef GZ4D_AV
#include <gz4d/av/AVWriter.h>
#endif
#include <gz4d/glo/FramebufferObject.h>
#include <gz4d/Image.h>

class VideoRecorder
{
public:
    VideoRecorder();
    ~VideoRecorder();

    bool Init();
    void AddFrame(float weight, int startLine = -1, int endLine = -1, bool commit = true);
    void WriteFrame();
    void Done();
    bool isRecordedFrame(){return recordedFrameFlag;}
    void setRecordedFrame(bool flag=true){recordedFrameFlag = flag;}
    bool useWatermark(){return watermark;}

    void bind();
    void unbind();

    unsigned long getWidth(){return w;}
    unsigned long getHeight(){return h;}

    bool setParameter(int argc, const char * argv[]);
    std::string getParameter(int argc, const char * argv[]);

private:

    unsigned char * data;
    //unsigned char * frame;
    gz4d::ImageRGBA8 frame;
    GLfloat * multiFrameBuffer;

    bfs::path filename;
    unsigned int videoBitrate;
    unsigned int audioBitrate;
    double spatialAA;
    bool watermark;

    double currentFrameTime;
    double currentSampleTime;
    int fps;
    double startTime, endTime;
    int frameno;
    unsigned int w,h;
    //unsigned long fbw,fbh;
    //float aa;

    bool recordingAudio;
    std::vector<short> audioData;

    bool recordedFrameFlag;
    bool preview;

    gz4d::glo::Framebuffer::Ptr osb;
    gz4d::glo::Renderbuffer::Ptr crb;
    gz4d::glo::Renderbuffer::Ptr drb;

    gz4d::glo::Framebuffer::Ptr msosb;
    gz4d::glo::Renderbuffer::Ptr mscrb;
    gz4d::glo::Renderbuffer::Ptr msdrb;

#ifdef GZ4D_AV
    gz4d::AVWriter avw;
#endif
};

#endif
