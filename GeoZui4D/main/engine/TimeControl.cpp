/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "engine/TimeControl.h"
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include"engine/Notes.h"

// #ifdef WIN32
// #include <sys/types.h>
// #include <sys/timeb.h>
// #else
// #include <sys/time.h>
// #include <unistd.h>
// #endif

#include<time.h>
#include "engine/ToglWindow.h"
#include "engine/RTSource.h"
#include <filesystem>
#include "engine/WindowMediator.h"
#include "utilities/TimePieces.h"

char * TimeControl::GZtimestr = NULL; // Time to be displayed
char * TimeControl::oldGZtimestr = NULL; // Last time to be displayed
double TimeControl::GZtimeFraction = 0.0;
double TimeControl::currentGZTime = 0.0;

int TimeControl::tcl = 0;

static int gztime_Cmd(ClientData clientData, Tcl_Interp *interp,
                       int argc, const char *argv[]);
                       
#include"MainWindow.h"
#include"engine/Scene3D.h"
extern MainWindow *mainWindow;

using namespace std;

//#include "TimeCircle.h"

TimeControl::TimeControl()
: ToglWindow("timecontrol")
{
//  state = PLAY;
    currentSysTime = lastSysTime = currentGZTime = lastGZTime = 0.0;
    offset = 0.0;
    scale = 1.0;
    spp = 1.0/50.0;
    
    bbox = NULL;

    if(!GZtimestr)
        GZtimestr = new char[126];
    if(!oldGZtimestr)
        oldGZtimestr = new char[126];

    oldGZtimestr[0] = GZtimestr[0] = '\0';
    timeScaleVisible = 0;

    usingRT = 0;

    frameTime = 0;

    Tcl_Eval("togl .timecontrol -depth true -stereo $useStereo -double true -sharecontext main -height 40 -ident timecontrol; pack .timecontrol -side bottom -fill x; global guiMode"); //; if {$guiMode == \"nav\"} {pack forget .timecontrol}
    Tcl_Eval("bind .timecontrol <MouseWheel> \"focus .timecontrol; if {%D > 0} {.timecontrol mouse %x %y %s 4 0} else {.timecontrol mouse %x %y %s 5 0}\"");
    Tcl_Eval("bind .timecontrol <ButtonPress> \"focus .timecontrol; .timecontrol mouse %x %y %s %b 0\"");
    Tcl_Eval("bind .timecontrol <ButtonRelease> \"focus .timecontrol; .timecontrol mouse %x %y %s %b 1\"");
    Tcl_Eval("bind .timecontrol <Motion>        \".timecontrol motion %x %y %s 0\"");
    RegisterWithTcl();

    windowMediator->PushWindow(this);

    paused = 0;
    loopAtEnd = GL_TRUE;
        
    initButtons();
    
    startX = -1;
    scrollClick = -1;
    looping = 0;
    loopEnd = loopStart = 0;
    tp = new TimePieces;
    
    
    targetTime = 0.0;
    mouseY = 0;
    zooming = 0;
    pageFwdStart = pageBackStart=0.0;
    tp->decompose(0.0,0.0);

    tstartp = new TimePieces;
    
    recording = 0;
    
    timezone = 0;  //default to UTC, but can be set from tcl using gztime timezone -5 (EST, for example)
    rt = NULL;
}

void TimeControl::RegisterWithTcl()
{
    if(!tcl)
    {
        Tcl_LinkVar("GZtimestr", (char *)&GZtimestr, TCL_LINK_STRING | TCL_LINK_READ_ONLY); //readonly added by mdp 11/29/2005
        Tcl_LinkVar("GZtime", (char *)&GZtimeFraction, TCL_LINK_DOUBLE);
        Tcl_LinkVar("currentGZtime", (char *)&currentGZTime, TCL_LINK_DOUBLE | TCL_LINK_READ_ONLY); //readonly added by mdp 11/29/2005
        tcl = 1;
    }
    
    Tcl_CreateCommand(interp, "gztime", gztime_Cmd,(ClientData) this, NULL);
}



void TimeControl::updateCurrentTime(int frameFlag)
{
//  char timeCmd[256];
    GLboolean recalc = GL_FALSE;

#ifdef WIN32
    timeb timebuffer;
    ftime(&timebuffer);
    currentSysTime = timebuffer.time+(timebuffer.millitm/1000.0);
#else
    timeval timebuffer;
    gettimeofday(&timebuffer,NULL);
    currentSysTime = timebuffer.tv_sec+(timebuffer.tv_usec/1000000.0);
#endif

    if(frameFlag)
    {
        frameTime = currentSysTime - lastSysTime;
        lastSysTime = currentSysTime;
    }

    if(isRT() && rt)
    {
        currentSysTime = rt->GetCurrentTime();
    }
    
    
    lastGZTime = currentGZTime;
    if(paused || startX>=0 || scrollClick >= 0 || recording) // if paused or draging with the mouse
        offset = lastGZTime-(currentSysTime*scale);
    currentGZTime = (currentSysTime*scale)+offset;

        
    mainWindow->getScene3D()->UpdateBBox();
    
    if(bbox) //&&& in the future, may only want to do this when frameFlag is GL_FALSE? (note by mdp 3/13/2003)
    {
        if(bbox->end == bbox->start)
        {
            offset = 0.0;
            scale = 1.0;
            recalc = GL_TRUE;
        }
        else
        {
            if(currentGZTime > bbox->end)
            {
                if (loopAtEnd)
                    offset = bbox->start - (currentSysTime*scale);
                else
                {
                    offset = bbox->end - (currentSysTime*scale);
                    paused = GL_TRUE;
                }
                recalc = GL_TRUE;
            }
            if(currentGZTime < bbox->start)
            {
                if (loopAtEnd)
                    offset = bbox->end - (currentSysTime*scale);
                else
                {
                    offset = bbox->start - (currentSysTime*scale);
                    paused = GL_TRUE;
                }
                recalc = GL_TRUE;
            }
            if(looping)
            {
                if(currentGZTime > loopEnd)
                {
                    offset = loopStart - (currentSysTime*scale);
                    recalc = GL_TRUE;
                }
                if(currentGZTime < loopStart)
                {
                    offset = loopEnd - (currentSysTime*scale);
                    recalc = GL_TRUE;
                }
            }

        }
        
        if (recalc) //recalc added by mdp 4/11/2003, moved outside if's 4/14/2003 :)
            currentGZTime = (currentSysTime*scale)+offset;

        if(bbox->end > bbox->start)
        {
            GZtimeFraction = (currentGZTime-bbox->start)/(bbox->end-bbox->start);
        }
        else
        {
            GZtimeFraction = 1.0;
        }
        Tcl_UpdateLinkedVar("GZtime");

/*      if(!timeScaleVisible && ( bbox->end!=0 || bbox->start!=0))
        {
            sprintf(timeCmd,"global mainWindow; pack .timeframe -side bottom -fill x -before $mainWindow");
            Tcl_Eval(timeCmd);
            timeScaleVisible = 1;
        }*/
    }
    tp->decompose(currentGZTime,timezone);
    tp->getString(GZtimestr,timezone, scale);

    if(strcmp(GZtimestr,oldGZtimestr))
    {
        strcpy(oldGZtimestr,GZtimestr);
        if(tcl)
            Tcl_UpdateLinkedVar("GZtimestr");
        if (!mainWindow->isRecording())
            Togl_PostRedisplay(myTogl);
    }

    Tcl_UpdateLinkedVar("currentGZtime");
}

void TimeControl::getSysTimeStr(char *tstr)
{
    time_t t;
    t = (time_t)currentSysTime;
    strcpy(tstr,ctime(&t));
    tstr[strlen(tstr)-1]=0; //get rid of \n
}

void TimeControl::getTimeStr(char *tstr,double tm)
{
    time_t t;
    t = (time_t)tm;
    strcpy(tstr,ctime(&t));
    tstr[strlen(tstr)-1]=0; //get rid of \n
}


void TimeControl::print(double dt)
{
    time_t t;
    t = (time_t)dt;
    cerr << ctime(&t) << endl;
}

void TimeControl::setBBox(BBox * ptrBBox)
{
    bbox = ptrBBox;
}

void TimeControl::setGZTime(double time, int fractionFlag)
{
    currentGZTime = time;
    updateCurrentTime(); //added by mdp 4/17/2003: update GeoZui's current notion of time first!
    if(fractionFlag)
    {
        if(bbox)
        {
            offset = (bbox->start+(time*(bbox->end-bbox->start))) - (currentSysTime*scale);
        }
    }
    else
    {
    offset = time - (currentSysTime*scale);
  }
}

void TimeControl::stop()
{
//    state = STOP;
    //scale = 0.0;
    paused = GL_TRUE;
    //offset = currentGZTime;
    
}

void TimeControl::play()
{
//    state = PLAY;
    scale = 1.0;
    offset = currentGZTime-currentSysTime;
    if (usingRT)
        Scene3D::setContinuousChange(false);
    usingRT=0;
}

void TimeControl::rev()
{
    scale = -scale;
    offset = currentGZTime-(currentSysTime*scale);
}

void TimeControl::multiplyScale(double mult)
{
    scale *= mult;
     if(fabs(scale) < 0.000001)
     {
        if(scale > 0)
            scale = 0.000001;
        else
            scale = -0.000001;
     }
    offset = currentGZTime-(currentSysTime*scale);
    //cerr << "Time scale: " << scale << endl;
}

void TimeControl::setTimeAndScale(double t,double s)
{
    scale = s;
    if(fabs(scale) < 0.000001)
    {
        if(scale > 0)
            scale = 0.000001;
        else
            scale = -0.000001;
    }
    setGZTime(t);
}

void TimeControl::setOffsetAndScale(double o,double s)
{
    scale = s;
    offset = o;
}

static int gztime_Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv)
{
    TimeControl * tc = (TimeControl*)clientData;
    int i;

    if(argc < 2)
    {
        return TCL_ERROR;
    }
    
    if(strcmp(argv[1],"timescale")==0)
    {
        if(argc == 3)
        {
            tc->setGZTime(atof(argv[2]),1);
            return TCL_OK;
        }
        else if(argc == 2)
        {
                char s[1024];
                sprintf(s,"%f",tc->getScale());
                Tcl_AppendResult(interp,s,NULL);
                return TCL_OK;
        }
        else
        {
                Tcl_AppendResult(interp,"Usage: gztime timescale [newscale]",NULL);
            return TCL_ERROR;
        }
    }
    
    if(strcmp(argv[1],"play")==0)
    {
        tc->play();
        return TCL_OK;
    }
    
    if(strcmp(argv[1],"stop")==0)
    {
        tc->stop();
        return TCL_OK;
    }


    if(strcmp(argv[1],"rev")==0)
    {
        tc->rev();
        return TCL_OK;
    }

        if(strcmp(argv[1],"repeat")==0)
    {
        tc->repeat();
        return TCL_OK;
    }

        if(strcmp(argv[1],"dontrepeat")==0)
    {
        tc->dontRepeat();
        return TCL_OK;
    }

        if(strcmp(argv[1],"rt")==0)
    {
        tc->useRT();
        return TCL_OK;
    }

        if(strcmp(argv[1],"endloop")==0)
    {
        tc->setLoopEnd();
        return TCL_OK;
    }

        if(strcmp(argv[1],"loop")==0)
    {
        tc->loop();
        return TCL_OK;
    }

        if(strcmp(argv[1],"dontloop")==0)
    {
        tc->dontLoop();
        return TCL_OK;
    }

        if(strcmp(argv[1],"startloop")==0)
    {
        tc->setLoopStart();
        return TCL_OK;
    }

    if(strcmp(argv[1],"multscale")==0)
    {
        if(argc == 3)
        {
            tc->multiplyScale(atof(argv[2]));
            return TCL_OK;
        }
        return TCL_ERROR;
    }

    if(strcmp(argv[1],"timezone")==0)
    {
        if(argc == 3)
        {
            tc->setTimezone(atof(argv[2]));
            return TCL_OK;
        }
        return TCL_ERROR;
    }

    if(strcmp(argv[1],"get")==0)
    {
        char time[64];
        
        sprintf(time,"%f",tc->getGZTime());
        //cerr << time << endl;
        Tcl_AppendResult(interp,time,NULL);
        return TCL_OK;
    }
    
    if(strcmp(argv[1],"set")==0)
    {
        double scale;
        if(argc == 4)
            scale = atof(argv[3]);
        else
            scale = tc->getScale();
        if(argc>=3)
        {
            tc->setTimeAndScale(atof(argv[2]),scale);
            return TCL_OK;
        }
        return TCL_ERROR;
    }

     
    for(i=0;i<argc;i++)
    {
        cerr << argv[i] << " ";
    }
    cerr << endl;
    cerr << "not yet supported\n";
    cerr << "Choose from timescale, play, stop, rev, repeat, dontrepeat, rt, endloop,\n";
    cerr << "loop, dontloop, startloop, multscale, timezone, get, and set.\n";
    return TCL_OK;
}


int TimeControl::isLeap(long y)
{
    if(y%4)
    {
        return 0;

    }
    else if (y%400==0)
    {
        return 1;
    }
    else if (y%100==0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void TimeControl::loop()
{
    if(loopStart!=loopEnd)
    {
        looping = 1;
    }
}

void TimeControl::setLoopStart()
{
    loopStart = currentGZTime;
}

void TimeControl::setLoopEnd()
{
    loopEnd = currentGZTime;
}

void TimeControl::Display()
{
    char tstring[50];
    int noTimeSpan = 0;
    GLint depthFunc;
    
    Notes * notes = mainWindow->GetNotes();

    updateCurrentTime();
    if (!mainWindow->isRecording())
    {
        SetTogl(myTogl);
        glDrawBuffer(GL_BACK);
        glClearColor(0.1f,0.1f,0.2f,0.0);
    }
    else
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glClearColor(0.0f,0.0f,0.0f,0.0);
    }
    if(!(bbox->start<bbox->end))
        noTimeSpan = 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_LIGHTING);
    int now = width/2;
    
    glLineWidth(2.0);
    
    
    spp = fabs(scale/50.0);  //seconds per pix
    //cerr << "spp: " << spp << endl;
    
    double tstart = lastGZTime-now*spp;
    float halfHeight = 0.5 * height, fullHeight = (float)height, ftemp;

    tp->getString(tstring, timezone, scale);

    glColor3f(1.0,1.0,1.0);
    int labelWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18,(unsigned char *)tstring)+2;
    if(labelWidth < 240)
        labelWidth = 240;
    
    int labelStart = width - labelWidth;
    printAt(labelStart, 2, 0, tstring, GL_FALSE,GLUT_BITMAP_HELVETICA_18);

    if(!noTimeSpan)
    {
        glBegin(GL_LINES);
            glColor3f(0.8f,0.8f,0.8f);
            glVertex2f(0.0, fullHeight);
            glVertex2f(width, fullHeight);
        glEnd();
    
    
        tstartp->decompose(tstart,timezone);

        tickColor[0] = 0.0;
        tickColor[1] = 0.9f;
        tickColor[2] = 0.0;
    
        bgColor[0] = 0.1f;
        bgColor[1] = 0.1f;
        bgColor[2] = 0.2f;
    
        textColor[0] = 1.0;
        textColor[1] = 1.0;
        textColor[2] = 1.0;

        drawTimeScale(height*.5,height,0.0,width);
        
        glColor3f(0.3f,0.3f,0.9f);
        glLineWidth(3);
        glBegin(GL_LINES);
            glVertex2d(now,height);
            glVertex2d(now,0.5*height);
        glEnd();
        glLineWidth(2);
    
        //bbox time display
        
        glColor4f(0.9f,0.3f,0.3f,0.4f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        ftemp = (float)(now - (lastGZTime-bbox->start) / spp);
        if (ftemp > 0)
        {
            glBegin(GL_QUADS);
                glVertex2f(0.0, halfHeight);
                glVertex2f(ftemp, halfHeight);
                glVertex2f(ftemp, fullHeight);
                glVertex2f(0.0, fullHeight);
            glEnd();
        }
        ftemp = (float)(now - (lastGZTime-bbox->end) / spp);
        if (ftemp < width)
        {
            glBegin(GL_QUADS);
                glVertex2f(ftemp, halfHeight);
                glVertex2f(width, halfHeight);
                glVertex2f(width, fullHeight);
                glVertex2f(ftemp, fullHeight);
            glEnd();
        }
        glDisable(GL_BLEND);
        
        // buttons
        glColor3f(0.0f,0.9f,0.0f);
        int buttonsEnd = 2; 

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glRasterPos2i(2,2);
    
        if(scale < 0 && !paused)
            glBitmap(24,16,0,0,28,0,buttonReverseActive);
        else
            glBitmap(24,16,0,0,28,0,buttonReverse);
        buttonsEnd += 28;
    
        if(!paused)
            glBitmap(24,16,0,0,28,0,buttonUnpaused);
        else
            glBitmap(24,16,0,0,28,0,buttonPaused);
        buttonsEnd += 28;
    
        if(scale > 0 && !paused)
            glBitmap(24,16,0,0,28,0,buttonForwardActive);
        else
            glBitmap(24,16,0,0,28,0,buttonForward);
        buttonsEnd += 28;

        
        if(scale == 1.0 && !paused)
            glBitmap(24,16,0,0,28,0,buttonOneActive);
        else
            glBitmap(24,16,0,0,28,0,buttonOneInactive);
        buttonsEnd += 28;
        
        if(rt)
        {       
            if(isRT())
                glBitmap(24,16,0,0,28,0,buttonRTActive);
            else
                glBitmap(24,16,0,0,28,0,buttonRTInactive);
            buttonsEnd += 28;
        }

        scrollAreaStart = buttonsEnd + 2; 
        scrollAreaEnd = labelStart - 2;
        
        scrollAreaWidth = scrollAreaEnd - scrollAreaStart;;
    

        // scroll bar
        scrollStart = scrollAreaStart + (int)(((tstart-bbox->start) /(bbox->end-bbox->start))*scrollAreaWidth);
        scrollEnd = scrollAreaStart + (int)((((lastGZTime+now*spp)-bbox->start)/(bbox->end-bbox->start))*scrollAreaWidth);
        scrollMiddle = scrollStart + (scrollEnd-scrollStart)/2;
    
        tstartp->decompose(bbox->start,timezone);
        spp = (bbox->end-bbox->start)/scrollAreaWidth;
        
        tickColor[0] = 0.0;
        tickColor[1] = 0.9f;
        tickColor[2] = 0.0;
    
        bgColor[0] = 1.0;
        bgColor[1] = 1.0;
        bgColor[2] = 1.0;
    
        textColor[0] = 0.1f;
        textColor[1] = 0.1f;
        textColor[2] = 0.2f;

        glColor3fv(bgColor);    
        glBegin(GL_QUADS);
            glVertex2f(scrollAreaStart,height*0.5);
            glVertex2f(scrollAreaStart,0.0);
            glVertex2f(scrollAreaEnd,0.0);
            glVertex2f(scrollAreaEnd,height*0.5);
        glEnd();
        
        drawTimeScale(0,height*0.5,scrollAreaStart,scrollAreaEnd);
        
        double nt;
        int x;
        
        glLineWidth(4.0);
        glColor4f(1.0,0.5,0.5,0.5);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        
        notes->ResetCursor();
        while((nt=notes->GetNextTime()))
        {
            x = (int)(scrollAreaStart+((nt-bbox->start)/(bbox->end - bbox->start))*(scrollAreaEnd-scrollAreaStart));
            glBegin(GL_LINES);
            glVertex2d(x,0.2*height);
            glVertex2d(x,0.5*height);
            glEnd();
        }
        glDisable(GL_BLEND);
        glLineWidth(2.0);
        
        int pad;
        if (scrollEnd - scrollStart < 30)
        {
            pad = (30 - (scrollEnd - scrollStart))/2;
            scrollStart-=pad;
            scrollEnd+=pad;
        }
    
        if(scrollStart < scrollAreaStart)
            scrollStart = scrollAreaStart;
        else if(scrollStart > scrollAreaEnd)
            scrollStart = scrollAreaEnd;
        if(scrollEnd < scrollAreaStart)
            scrollEnd = scrollAreaStart;
        else if(scrollEnd > scrollAreaEnd)
            scrollEnd = scrollAreaEnd;
    
        glColor4f(0.0f, 0.9f, 0.0f, 0.5f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glBegin(GL_QUADS);
            glVertex2f(scrollStart, 0.0);
            glVertex2f(scrollEnd, 0.0);
            glVertex2f(scrollEnd, halfHeight);
            glVertex2f(scrollStart, halfHeight);
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        glBegin(GL_LINES);
            glVertex2f(scrollMiddle, 0.0);
            glVertex2f(scrollMiddle, halfHeight);
        glEnd();
        glDisable(GL_BLEND);
        
        spp = fabs(scale/50.0);  //seconds per pix

    }
    glDepthFunc(depthFunc);
    glEnable(GL_LIGHTING);

    if (!mainWindow->isRecording())
    {
        Togl_SwapBuffers(myTogl);
    }
//  mainWindow->getScene3D()->timeCircle->Display();
}

void TimeControl::drawTimeScale(float bottom, float top, float left, float right)
{

    double sec;
    int min, hour, mday, mon, wday, yday;
    long year;
    
    double startPosition;
    int startTime;

    glBegin(GL_LINES);
        glColor3fv(tickColor);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
    glEnd();

    
    sec = tstartp->getSeconds();
    min = tstartp->getMinutes();
    hour = tstartp->getHour();
    mday = tstartp->getDayOfMonth();
    mon = tstartp->getMonth();
    year = tstartp->getYear();
    wday = tstartp->getDayOfWeek();
    yday = tstartp->getDayOfYear();
    
    const double secondsInADay = 3600.0 * 24.0, secondsInAYear = 3600.0 * 24.0 * 365.25; //secondsInAYear is approximate
    double temp;
    double secondsIntoDay = hour * 3600 + min * 60 + sec;
    double secondsIntoYear = yday * secondsInADay + secondsIntoDay;
    
    //miliseconds
    temp = sec * 1000.0;
    startPosition = left + (.001 - (temp - floor(temp))/1000.0)/spp;
    startTime = (int)ceil(temp) % 1000;
    drawTicks(.001,startPosition,startTime,1000,"%ims",50,bottom,top,left,right);
        
    //deciseconds
    temp = sec * 10.0;
    startPosition = left + (.1 - (temp - floor(temp))/10.0)/spp;
    startTime = (int)ceil(temp) % 10;
    drawTicks(.1,startPosition,startTime,10,"%ids",5,bottom,top,left,right);
    
    //seconds
    startPosition = left + (1-(sec - floor(sec)))/spp;
    startTime = (int)ceil(sec);
    drawTicks(1,startPosition,startTime,60,"%is",15,bottom,top,left,right);
    
    //minutes
    startPosition = left + (60.0 - sec)/spp;
    startTime = min + 1;
    drawTicks(60,startPosition,startTime,60,"%im",15,bottom,top,left,right);

    //hours
    startPosition = left + (3600.0-(min * 60 + sec))/spp;
    startTime = hour + 1;
    drawTicks(3600,startPosition,startTime,24,"%ih",6,bottom,top,left,right);

    //days
    startPosition = left + (secondsInADay - secondsIntoDay)/spp;
    startTime = wday + 1;
    drawTicks(secondsInADay,startPosition,startTime,7,"d",0,bottom,top,left,right);
    
    int daysInMonth = tstartp->getDaysInMonth();
    
    //months
    startPosition = left + ((daysInMonth * secondsInADay)-((mday - 1) * secondsInADay + secondsIntoDay))/spp;
    startTime = mon + 1;
    drawTicks(365.25/12.0 * secondsInADay,startPosition,startTime,12,"m",0,bottom,top,left,right);
    
    //years
    startPosition = left + (secondsInAYear - secondsIntoYear)/spp;
    startTime = year + 1;
    drawTicks(secondsInAYear,startPosition,startTime,0,"y%i",25,bottom,top,left,right);
    
    //kyears
    if(year > 0)
    {
        startPosition = left + (1000.0 * secondsInAYear - ((year % 1000) * secondsInAYear + secondsIntoYear))/spp;
    }
    else
    {
        startPosition = left + (-year % 1000 * secondsInAYear + secondsInAYear - secondsIntoYear)/spp;
    }
    startTime = (int)ceil(year / 1000.0);
    drawTicks(1000 * secondsInAYear,startPosition,startTime,0,"y%ik",10,bottom,top,left,right);

    //myears
    if(year > 0)
    {
        startPosition = left + (1000000.0 * secondsInAYear - ((year % 1000000) * secondsInAYear + secondsIntoYear))/spp;
    }
    else
    {
        startPosition = left + (-year % 1000000 * secondsInAYear + secondsInAYear - secondsIntoYear)/spp;
    }
    startTime = (int)ceil(year / 1000000.0);
    drawTicks(1000000 * secondsInAYear,startPosition,startTime,0,"y%im",10,bottom,top,left,right);

    //gyears
    if(year > 0)
    {
        startPosition = left + (1000000000.0 * secondsInAYear -((year % 1000000000) * secondsInAYear + secondsIntoYear))/spp;
    }
    else
    {
        startPosition = left + (-year % 1000000000 * secondsInAYear + secondsInAYear - secondsIntoYear)/spp;
    }
    startTime = (int)ceil(year / 1000000000.0);
    drawTicks(1000000000 * secondsInAYear,startPosition,startTime,0,"y%ig",10,bottom,top,left,right);

}

/*  TimeControl::drawTicks - what the hay do those params mean???

    interval is the interval in seconds between the tick marks,
    start position is the position of the first tick mark, in pixels
    spp is seconds-per-pixel, to know how far to jump, in pixels, to
        the next tick mark
    start time is the whole number representint the time, in the
        desired unit, of the first tick mark
    loopLimit tells us how many of those whole time values exists
        so we know when to rollover to 0 (if 0, no roll over, such as years)
    displayFormat has two puposes. In the case where the ticks represent numbers,
        this contains the printf format (with one int) to display with unit etc
        If it is a 1 char string, the char tells us if the time needs to be
        converted to a day or month string instead of displaying a numerical value
    subInterval tells us what interval to use to display if it gets too crowded for
        all values to be displayed.
*/
void TimeControl::drawTicks(double interval, double startPosition, int startTime, int loopLimit, char const * displayFormat, int subInterval, float bottom, float top, float left, float right)
{
    double i;
    int t,l;
    char tstring[50], c;
    
    double pixInterval = interval/spp;
    if(pixInterval < 3 || pixInterval >= 100000)
        return;
    //double tickScale = pixInterval/300.0;
    double tickScale = 0.8*(1.0- 3.0/pixInterval);
    if(tickScale>1.0)
        tickScale=1.0;
    for(i=startPosition, t=startTime;i<right;i+=interval/spp, t++)
    {
        if(i>left)
        {
            if(loopLimit && t>=loopLimit)
                t=0;
            glBegin(GL_LINES);
                glVertex2f((float)i, bottom);
                glVertex2f((float)i, bottom + tickScale*(top-bottom) );
            glEnd();
            if(pixInterval > 50)
            {
                glColor3fv(textColor);
                if(displayFormat[1]==0)
                {
                    c = displayFormat[0];
                    if (c == 'd')
                        sprintf(tstring,"%s",TimePieces::getDayStr(t));
                    if (c == 'm')
                        sprintf(tstring,"%s",TimePieces::getMonthStr(t));
                }
                else
                    sprintf(tstring,displayFormat,t);

                l = glutBitmapLength(GLUT_BITMAP_HELVETICA_18,(unsigned char *)tstring);
                glBegin(GL_QUADS);
                glColor3fv(bgColor);
                glVertex2f((float)(i + 1.0), bottom + 0.9f * (top-bottom));
                glVertex2f((float)(i + 1.0), bottom + 0.1f * (top-bottom));
                glVertex2f((float)(i + l + 6.0), bottom + 0.1f * (top-bottom));
                glVertex2f((float)(i + l + 6.0), bottom + 0.9f * (top-bottom));
                glEnd();
                glColor3fv(textColor);
                printAt(i+3,bottom+.1*(top-bottom),0.0,tstring,GL_FALSE,GLUT_BITMAP_HELVETICA_18);
                glColor3fv(tickColor);
            }
            else if(subInterval*pixInterval > 50 && displayFormat[1] && !(t%subInterval))
            {
                glColor3fv(textColor);
                sprintf(tstring,displayFormat,t);
                printAt(i+3,bottom+.1*(top-bottom),0.0,tstring,GL_FALSE,GLUT_BITMAP_HELVETICA_12);
                glColor3fv(tickColor);
            }
        }
    }   
}

void TimeControl::Reshape(int x, int y, int w, int h)
{
    ToglWindow::Reshape(x,y,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(x,width,y,height,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void TimeControl::useRT()
{
    if (!usingRT)
        Scene3D::setContinuousChange(true);
    usingRT = 1;
    scale=1.0;
    offset=0.0;
}

int TimeControl::isRT()
{
    if(usingRT && scale==1.0 && offset==0.0)
        return 1;
    return 0;
}

void TimeControl::setTimezone(float t)
{
    //cerr << "TimeControl::setTimezone: " << t << endl;
    timezone = t;
}

int TimeControl::HandleInput(unsigned int inputType, InputInfo *info)
{
    double tscale;

    double tmpscale;

    if (inputType == NoInput)
    {
        if(targetTime)
        {
            if (((currentSysTime-startTranslateSysTime)/0.25)>1.0)
            {
                setGZTime(targetTime);
                targetTime = 0.0;
            }
            else
            {
                setGZTime(startTranslateGZTime+(targetTime-startTranslateGZTime)*((currentSysTime-startTranslateSysTime)/0.25));
            }
        }
        if(zooming)
        {
            tscale = 1+1*(currentSysTime-lastZoomTime);
            if(zooming > 0)
                multiplyScale(tscale);
            else
                multiplyScale(1/tscale);
            lastZoomTime = currentSysTime;
        }
        if(pageBackStart && pageBackStart < currentSysTime-0.5)
            setGZTime(currentGZTime-(width*spp));
        if(pageFwdStart && pageFwdStart < currentSysTime-0.5)
            setGZTime(currentGZTime+(width*spp));
        
        Display();
    }
    
    if( inputType & MouseButtonDown && info->val.i >= 4 && info->val.i <= 5 ) // Scroll wheeel
    {
        if(info->modifier.i & ModControl)
            if(info->val.i == 5)
                multiplyScale(1.0/0.6);
            else
                multiplyScale(0.6);
        else if(info->modifier.i & ModShift)
            if(info->val.i == 5)
                multiplyScale(1.0/0.95);
            else
                multiplyScale(0.95);
        else
            if(info->val.i == 5)
                multiplyScale(1.0/0.8);
            else
                multiplyScale(0.8);
        return SINK_EVENT;
    }
    
    if( inputType & MouseButtonDown && info->val.i == 1)  // left click
    {
        // remember, mouse pos is relative to upper left, while 0,0 of draw space is lower left
        if(info->y.i >20) // are we in the lower part, where the buttons are?
        {
            if(info->x.i<28)
            {
                if(scale>0)
                    rev();
                if(paused)
                    paused = !paused;
            }
            if(info->x.i>=28 && info->x.i < 56)
                paused = !paused;
            if(info->x.i>=56 && info->x.i < 84)
            {
                if(scale<0)
                    rev();
                if(paused)
                    paused = !paused;
            }
            if(info->x.i>=84 && info->x.i < 112)
            {
                scale = 1.0;
                offset = currentGZTime-currentSysTime;
                if(paused)
                    paused = !paused;
            }
            if(rt && info->x.i>=112 && info->x.i < 140)
            {
                useRT();
            }
            if(info->x.i>=scrollStart && info->x.i <= scrollEnd)
            {
                scrollClick = info->x.i;
                startTime = currentGZTime;
            }
            else if(info->x.i>=scrollAreaStart && info->x.i <= scrollAreaEnd)
            {
                if(info->x.i < scrollStart)
                {
                    pageBackStart = currentSysTime;
                    setGZTime(currentGZTime-(width*spp));
                }
                else
                {
                    pageFwdStart = currentSysTime;
                    setGZTime(currentGZTime+(width*spp));
                }
            }
        }
        else
        {
            startX = info->x.i;
            startY = info->y.i;
            startTime = currentGZTime;

            startScale = scale;
        }
        return SINK_EVENT;
    }
    
    // middle mouse
    if( inputType & MouseButtonDown && info->val.i == 2)
    {
        // remember, mouse pos is relative to upper left, while 0,0 of draw space is lower left
        if(info->y.i <20) // are we in the time scale part
        {
            targetTime = currentGZTime + spp*(info->x.i-(width/2));
            startTranslateSysTime = currentSysTime;
            startTranslateGZTime = currentGZTime;
            mouseY = info->y.i;
            lastZoomTime = currentSysTime;
        }
    }

    if( inputType & MouseButtonUp && info->val.i == 2)
    {
        targetTime = 0.0;
        mouseY = 0;
        zooming = 0;
    }
    
    if( inputType & MouseButtonUp && info->val.i == 1)
    {
        if(startX >= 0)

        {
            startX = -1;
            setTimeAndScale(getGZTime(),startScale);
        }
        if(scrollClick >= 0)
            scrollClick = -1;
        pageBackStart = pageFwdStart = 0.0;
    }
    
    if(inputType & MouseMotion)
    {
        if(startX >= 0)
        {
            if(abs(info->y.i-startY)>100)
            {
                tmpscale = 1+(abs(info->y.i-startY)-100)/3.0;
            }
            else
                tmpscale = 1;

            startTime = startTime - ((info->x.i-startX)*spp);
            startX = info->x.i;
            setTimeAndScale(startTime,startScale*tmpscale);
        }
        if(scrollClick >= 0)
        {
            setGZTime(startTime + ((info->x.i-scrollClick)*(bbox->end-bbox->start))/scrollAreaWidth);
        }
        if(mouseY)
        {
            if(info->y.i > mouseY+10)
                zooming = 1;
            else if (info->y.i < mouseY-10)
                zooming = -1;
            else zooming = 0;
        }
        
    }
    
    return ToglWindow::HandleInput(inputType,info);
}

void TimeControl::initButtons()
{

    static GLubyte p[] = {1,2,4,8,16,32,64,128};
    int i,j;
    for(i=0;i<48;i++)
    {
        buttonPaused[i]=0;
        buttonUnpaused[i]=0;
        buttonReverse[i] = 0;
        buttonForward[i] = 0;
        buttonReverseActive[i] = 0;
        buttonForwardActive[i] = 0;
        buttonRTActive[i] = 0;
        buttonRTInactive[i] = 0;
        buttonOneActive[i] = 0;
        buttonOneInactive[i] = 0;
    }
    static char unpause[16][25] ={" ********************** ",
                                "************************",
                                         "**                    **",
                                         "**    ****    ****    **",
                                         "**   **  **  **  **   **",
                                         "**   **  **  **  **   **",
                                         "**   **  **  **  **   **",
                                         "**   **  **  **  **   **",
                                         "**   **  **  **  **   **",
                                         "**   **  **  **  **   **",
                                         "**   **  **  **  **   **",
                                         "**   **  **  **  **   **",
                                         "**    ****    ****    **",
                                         "**                    **",
                                         "************************",
                                         " ********************** "};
    
    for(i=0;i<16; i++)
        for(j=0;j<8;j++)
        {
            if(unpause[15-i][j]=='*')
                buttonUnpaused[i*3]|=p[7-j];
            if(unpause[15-i][j+8]=='*')
                buttonUnpaused[i*3+1]|=p[7-j];
            if(unpause[15-i][j+16]=='*')
                buttonUnpaused[i*3+2]|=p[7-j];
        }

    static char pause[16][25] ={" ********************** ",
                                "************************",
                                         "**                    **",
                                         "**    ****    ****    **",
                                         "**   ******  ******   **",
                                         "**   ******  ******   **",
                                         "**   ******  ******   **",
                                         "**   ******  ******   **",
                                         "**   ******  ******   **",
                                         "**   ******  ******   **",
                                         "**   ******  ******   **",
                                         "**   ******  ******   **",
                                         "**    ****    ****    **",
                                         "**                    **",
                                         "************************",
                                         " ********************** "};
    
    for(i=0;i<16; i++)
        for(j=0;j<8;j++)
        {
            if(pause[15-i][j]=='*')
                buttonPaused[i*3]|=p[7-j];
            if(pause[15-i][j+8]=='*')
                buttonPaused[i*3+1]|=p[7-j];
            if(pause[15-i][j+16]=='*')
                buttonPaused[i*3+2]|=p[7-j];
        }
        
    static char revIn[16][25] ={" ********************** ",
                                "************************",
                                         "**                    **",
                                         "**            ***     **",
                                         "**          *****     **",
                                         "**        ***  **     **",
                                         "**      ***    **     **",
                                         "**    ***      **     **",
                                         "**    ***      **     **",
                                         "**      ***    **     **",
                                         "**        ***  **     **",
                                         "**          *****     **",
                                         "**            ***     **",
                                         "**                    **",
                                         "************************",
                                         " ********************** "};
    
    for(i=0;i<16; i++)
        for(j=0;j<8;j++)
        {
            if(revIn[15-i][j]=='*')
            {
                buttonReverse[i*3]|=p[7-j];
                buttonForward[i*3+2]|=p[j];
            }
            if(revIn[15-i][j+8]=='*')
            {
                buttonReverse[i*3+1]|=p[7-j];
                buttonForward[i*3+1]|=p[j];
            }
            if(revIn[15-i][j+16]=='*')
            {
                buttonReverse[i*3+2]|=p[7-j];
                buttonForward[i*3]|=p[j];
            }
        }

    static char revAc[16][25] ={" ********************** ",
                                "************************",
                                         "**                    **",
                                         "**            ***     **",
                                         "**          *****     **",
                                         "**        *******     **",
                                         "**      *********     **",
                                         "**    ***********     **",
                                         "**    ***********     **",
                                         "**      *********     **",
                                         "**        *******     **",
                                         "**          *****     **",
                                         "**            ***     **",
                                         "**                    **",
                                         "************************",
                                         " ********************** "};
    
    for(i=0;i<16; i++)
        for(j=0;j<8;j++)
        {
            if(revAc[15-i][j]=='*')
            {
                buttonReverseActive[i*3]|=p[7-j];
                buttonForwardActive[i*3+2]|=p[j];
            }
            if(revAc[15-i][j+8]=='*')
            {
                buttonReverseActive[i*3+1]|=p[7-j];
                buttonForwardActive[i*3+1]|=p[j];
            }
            if(revAc[15-i][j+16]=='*')
            {
                buttonReverseActive[i*3+2]|=p[7-j];
                buttonForwardActive[i*3]|=p[j];
            }
        }
        
    static char rtAc[16][25] ={" ********************** ",
                                "************************",
                                         "**                    **",
                                         "**   *****   ******   **",
                                         "**   **  **    **     **",
                                         "**   **  **    **     **",
                                         "**   *****     **     **",
                                         "**   ** **     **     **",
                                         "**   **  **    **     **",
                                         "**   **  **    **     **",
                                         "**                    **",
                                         "**   *************    **",
                                         "**  ***************   **",
                                         "**                    **",
                                         "************************",
                                         " ********************** "};
    
    for(i=0;i<16; i++)
        for(j=0;j<8;j++)
        {
            if(rtAc[15-i][j]=='*')
                buttonRTActive[i*3]|=p[7-j];
            if(rtAc[15-i][j+8]=='*')
                buttonRTActive[i*3+1]|=p[7-j];
            if(rtAc[15-i][j+16]=='*')
                buttonRTActive[i*3+2]|=p[7-j];
        }
        
    static char rtIn[16][25] ={" ********************** ",
                                "************************",
                                         "**                    **",
                                         "**   *****   ******   **",
                                         "**   **  **    **     **",
                                         "**   **  **    **     **",
                                         "**   *****     **     **",
                                         "**   ** **     **     **",
                                         "**   **  **    **     **",
                                         "**   **  **    **     **",
                                         "**                    **",
                                         "**   ***       ***    **",
                                         "**  *****     *****   **",
                                         "**                    **",
                                         "************************",
                                         " ********************** "};
    
    for(i=0;i<16; i++)
        for(j=0;j<8;j++)
        {
            if(rtIn[15-i][j]=='*')
                buttonRTInactive[i*3]|=p[7-j];
            if(rtIn[15-i][j+8]=='*')
                buttonRTInactive[i*3+1]|=p[7-j];
            if(rtIn[15-i][j+16]=='*')
                buttonRTInactive[i*3+2]|=p[7-j];
        }

    static char oneAc[16][25] ={" ********************** ",
                                "************************",
                                         "**                    **",
                                         "**                    **",
                                         "**    ***  ***   ***  **",
                                         "**   ****  ***  ****  **",
                                         "**  *****  *** *****  **",
                                         "**    ***        ***  **",
                                         "**    ***  ***   ***  **",
                                         "**    ***  ***   ***  **",
                                         "**    ***  ***   ***  **",
                                         "**   *****      ***** **",
                                         "**                    **",
                                         "**                    **",
                                         "************************",
                                         " ********************** "};
    
    for(i=0;i<16; i++)
        for(j=0;j<8;j++)
        {
            if(oneAc[15-i][j]=='*')
                buttonOneActive[i*3]|=p[7-j];
            if(oneAc[15-i][j+8]=='*')
                buttonOneActive[i*3+1]|=p[7-j];
            if(oneAc[15-i][j+16]=='*')
                buttonOneActive[i*3+2]|=p[7-j];
        }
        
    static char oneIn[16][25] ={" ********************** ",
                                "************************",
                                         "**                    **",
                                         "**                    **",
                                         "**    ***  ***   ***  **",
                                         "**   *  *  * *  *  *  **",
                                         "**  *** *  *** *** *  **",
                                         "**    * *        * *  **",
                                         "**    * *  ***   * *  **",
                                         "**    * *  * *   * *  **",
                                         "**    * *  ***   * *  **",
                                         "**   *****      ***** **",
                                         "**                    **",
                                         "**                    **",
                                         "************************",
                                         " ********************** "};
    
    for(i=0;i<16; i++)
        for(j=0;j<8;j++)
        {
            if(oneIn[15-i][j]=='*')
                buttonOneInactive[i*3]|=p[7-j];
            if(oneIn[15-i][j+8]=='*')
                buttonOneInactive[i*3+1]|=p[7-j];
            if(oneIn[15-i][j+16]=='*')
                buttonOneInactive[i*3+2]|=p[7-j];
        }
        
}
