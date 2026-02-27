/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include<gz4d/glo.h>
#include "Sound.h"
#include <iostream>
#include <GL/glut.h>
#include <boost/filesystem.hpp>

#include "MainWindow.h"
#include "engine/Scene3D.h"
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/ios.h>


REGISTER_OBJECT_WITH_MENU(Sound, "") = "{"
DEFAULT_OBJECT_MENU_ITEMS
"{Play} play "
"{Stop} stop "
"}";

#define SBUFFERSIZE 16384

extern MainWindow *mainWindow;


Sound * Sound::currentSound = NULL;
int Sound::recording = 0;
//bool Sound::soundInitialized = false;

std::shared_ptr<Sound::OutDevice> Sound::device;

Sound::Sound()
    :Object(GL_TRUE, PICK_CLICK1 | PICK_PRESS3 | PICK_DOUBLE1)
{
    //startTime = endTime = 0.0;
    sphereSize = 0.0;
    pointSize = 0;
    color[0]=color[1]=color[2]=1.0;
        
        
    playing = 0;
    //lastTime =0.0;
    //soundTime=0.0;
        
    soundBufferSize = SBUFFERSIZE;
        
    if(!device)
    {
        device = std::shared_ptr<OutDevice>(new OutDevice);
        
    }
/*    if(!soundInitialized)
    {
        currentSound = this;
        Sound_Init();
        Mix_HookMusic(musicHook,&currentSound);
        soundInitialized = true;
    }*/
    timeSituated = true;
        
    std::cerr << "created Sound with id: " << id << std::endl;
        
}

Sound::~Sound()
{
/*    if(currentSound == this)
        currentSound = NULL;*/
}

Object * Sound::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object::LoadXML(xml,path,ncfile);

    sphereSize = xml.get("<xmlattr>.spheresize",sphereSize);
    pointSize = xml.get("<xmlattr>.pointsize",pointSize);
    volume = xml.get("<xmlattr>.volume",volume);
    endTime = gz4d::Time(xml.get("<xmlattr>.endtime",endTime.secondsSinceEpoch().asSeconds()));

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.color"))
    {
        std::istringstream ss(*attribute);
        ss >> color[0];
        ss >> color[1];
        ss >> color[2];
    }

    boost::optional<double> double_attrib;
    if(double_attrib = xml.get_optional<double>("<xmlattr>.starttime"))
        startTime = gz4d::Time(*double_attrib);
    else
        timeSituated = false;

    return this;
}

void Sound::Idle(double time)
{
    if(time >= startTime.secondsSinceEpoch().asSeconds() && time <= endTime.secondsSinceEpoch().asSeconds())
    {
        if(!playing)
            play();
    }
    else if (timeSituated) //put in here to allow non-time-tagged sounds to play through (mdp 1/4/2008)
        stop();
                
    lastTime = gz4d::Time(time);
}

void Sound::DrawLowRes(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT);
    glColor3fv(color);
    if(pointSize)
    {
        glPointSize(pointSize);
        glBegin(GL_POINTS);
        glVertex3f(0.0,0.0,0.0);
        glEnd();
    }
    if(sphereSize)
    {
        glutSolidSphere(sphereSize,10,10);
    }
}


int Sound::pickAction (Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
    if(pickType == (PICK_DOUBLE1))
    {
        if(!playing)
        {
            play();
        }
        else
        {
            stop();
        }
    }
    return Object::pickAction(pickType, inputInfo, names, hits);
}

void Sound::play()
{
/*    if(sound && !playing)
    {
        cerr << "Play: " << id << endl;
        currentSound->stop();
        currentSound = this;
        playing = 1;
        //Sound_Rewind(sound);
        soundTime = 0.0;
    }*/
}

void Sound::stop()
{
    playing = 0;
}

void Sound::fillBufferFromCurrent(std::vector<int16_t> &buffer, double &t, double timescale)
{
    if(currentSound)
    {
        currentSound->fillBuffer(buffer,t,timescale);
    }
}

// version called by video recorder
void Sound::fillBuffer(std::vector<int16_t> &buffer, double &t, double timescale)
{
    //unsigned long int l;
    //long int pos;
/*    if(sound && playing)
    {
        pos = (long int)((t-startTime)*1000);
        Sound_Seek(sound,pos);
        if(len != soundBufferSize)
        {
            Sound_SetBufferSize(sound,len);
            soundBufferSize = len;
        }
        l = Sound_Decode(sound);
        if(len<l)
            memcpy(stream,sound->buffer,len);
        else
            memcpy(stream,sound->buffer,l);
        if(l<len)
        {
            memset(&(stream[l]),0,len-l);
        }
        t+=(timescale*l/(4.0*44100.0));
        if(sound->flags & SOUND_SAMPLEFLAG_EOF)
        {
            Sound_Rewind(sound);
            playing=0;
        }
    }
    else*/
    {
        memset(&buffer[0],0,buffer.size()*sizeof(int16_t));
        t+=(timescale*buffer.size()/(2.0*44100.0));
    }
}

Sound::OutDevice::OutDevice():gz4d::portaudio::Device(context.getDefaultOutputDevice())
{
        //gz4d::AudioDevice::Stream::DefaultOutputParameters op;
        gz4d::portaudio::Format f = context.getDefaultFormat();
        f.getOutputParameters().setChannelCount(2);
        outstream = std::shared_ptr<gz4d::portaudio::Stream>(new gz4d::portaudio::Stream(f.open()));
        
        //op.SetChannelCount(2);
        //outstream.Open(gz4d::AudioDevice::Stream::Format(gz4d::AudioDevice::Stream::Parameters(),op,44100),this);
        //outstream.Start();
        outstream->start();
        std::cerr << "Audio output: " << outstream->getSampleRate() << " Hz" << std::endl;
}

Sound::OutDevice::~OutDevice()
{
    outstream->stop();

}

bool Sound::OutDevice::ProduceData(float *buffer, unsigned long size)
{
    if (currentSound)
        currentSound->fillBuffer(buffer,size);	
    else
        memset(buffer,0,sizeof(float)*size*2);
    return true;
}

// version called by SDL hook
void Sound::fillBuffer(float * stream, unsigned long len)
{
/*    unsigned long int l;
    long int pos;
    if(playing && !recording)
    {
        if(sound)
        {
            if(lastTime-startTime>soundTime+.5 || lastTime-startTime < soundTime-.5)
            {
                pos = (long int)((lastTime-startTime)*1000);
                Sound_Seek(sound,pos);
                soundTime = lastTime-startTime;
            }
            if(len != soundBufferSize)
            {
                Sound_SetBufferSize(sound,len);
                soundBufferSize = len;
            }
            l = Sound_Decode(sound);
            memcpy(stream,sound->buffer,l);
            if(l<len)
            {
                memset(&(stream[l]),0,len-l);
            }
            soundTime+=(l/(4.0*44100.0));
            if(sound->flags & SOUND_SAMPLEFLAG_EOF)
            {
                Sound_Rewind(sound);
                playing=0;
            }
        }
    }
    else*/
        memset(stream,0,len*2*sizeof(float));
}

void Sound::ShowInfo(GLuint *names, int numNames)
{
    if(playing)
    {
        std::stringstream s;
        s << "Playing: Now at " << soundTime.asSeconds() << " of " << (endTime - startTime).asSeconds() << " seconds.\n";
        mainWindow->getScene3D()->appendInfo(s.str().c_str());
    }
    else
        mainWindow->getScene3D()->appendInfo("Not playing.\n");

    Object::ShowInfo(names, numNames);
}


// void musicHook(void *sound, Uint8 *stream, int len)
// {
//     Sound * s = *((Sound **)sound);
//     if (s)
//         s->fillBuffer(stream,(unsigned int)len);	
//     else
//         memset(stream,0,len);
// }


/*---------------------------------------------------------------------------*/
int Sound::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    if (argc >= 1)
    {
        std::string message = argv[0];
        if (message == "play")
        {
            play();
            return TCL_OK;
        }   
        if (message ==  "stop")
        {
            stop();
            return TCL_OK;
        }
    }
    return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Sound::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "play stop ", NULL);
    return Object::list(interp);
}
