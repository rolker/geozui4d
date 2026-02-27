/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef SOUND_H
#define SOUND_H

#include <gz4d/sound/Portaudio.h>
#include "engine/Object.h"
#include "engine/Interactor.h"
#include <queue>

class Sound : public Object, public Interactor
{
public:
    Sound();
    ~Sound();
    DERIVE_OBJECT(Sound);

    virtual Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    virtual void Idle(double time);
    void DrawLowRes(float degradation, Window3D *win);
    int pickAction (Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);
    virtual void play();
    void stop();
    virtual void fillBuffer(float * stream, unsigned long len);
    virtual void fillBuffer(std::vector<int16_t> &buffer,double &t, double timescale);
    static bool haveCurrentSound(){return(currentSound);}
    static void fillBufferFromCurrent(std::vector<int16_t> &buffer, double &t, double timescale);
    static void setRecording(int r){recording = r;};
    virtual void ShowInfo(GLuint *names, int numNames);

    int perform(int argc, const char **argv, Tcl_Interp *interp);
    int list(Tcl_Interp *interp);

protected:
    int playing;
    static int recording; // flag to tell us if we are recording a scene to a movie file
    gz4d::Time startTime,endTime;
    bool timeSituated;

    gz4d::time::Duration soundTime;
    gz4d::Time lastTime;

    class OutDevice: public gz4d::portaudio::Device
    {
        gz4d::portaudio::Context context;
        std::shared_ptr<gz4d::portaudio::Stream> outstream;
        public:
            OutDevice();
            ~OutDevice();
            bool ProduceData(float * buffer,  unsigned long size);
    };

    static std::shared_ptr<OutDevice> device;
    static Sound* currentSound;
private:

    float pointSize; 
    float sphereSize;
    float color[3];

    //Sound_Sample * sound;
    unsigned long int soundBufferSize;
    float volume;
};


#endif
