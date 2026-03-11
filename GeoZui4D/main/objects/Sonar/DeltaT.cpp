// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "DeltaT.h"
#include "GL/glut.h"
#include "utilities/TimePieces.h"
#include "engine/Scene3D.h"
#include "utilities/Utilities.h"
#include "engine/InputMediator.h"
#include <gz4d/glo/PushAttrib.h>

REGISTER_OBJECT(DeltaT, "837");
//REGISTER_ALIAS(DeltaT837, DeltaT, "837");

DeltaT::DeltaT()
    //:dft(500),bf_dft(256)
{
    frequencyRatio = 1.0;
    iux = NULL;
    ivx = NULL;
    data_id = 0;
    timeIndex = 0;
    needNewList = false;
    displayList = 0;
    beamCount = 120;
    swathAngle = M_PI*2.0/3.0;
    elementSpacing = 0.0028846; // 1500m/s / 260khz / 2
    //elementSpacing = 0.0057692; // 1 wavelength in meters at 260khz and 1500m/s
    binaryFile = true;
    xOffset = 0.0;
    showRaw = false;
    showBeamformedRectangle = false;
    showRawPhase = false;
    showRawBits = false;
    flipElements = false;
    ping.element.resize(16);
    for(std::vector<std::vector<double> >::iterator e = ping.element.begin(); e != ping.element.end(); ++e)
    {
        e->resize(500);
    }
    ping.element_frequency.resize(16);
    ping.synt_element.resize(16);
}

DeltaT::~DeltaT()
{
    delete iux;
    delete ivx;
}

void DeltaT::DrawLowRes(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT|GL_LIGHTING_BIT| GL_LINE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0,0.2,0.2);
    glPointSize(10);
    glTranslatef(xOffset,0.0,0.0);
    glBegin(GL_POINTS);
    glVertex3f(0.0,0.0,0.0);
    glEnd();
    if(needNewList && iux)
    {
        if(displayList == 0)
            displayList = glGenLists(1);
        glNewList(displayList,GL_COMPILE);
        glDisable(GL_LIGHTING);
        //glShadeModel(GL_FLAT);
        glDepthFunc(GL_LEQUAL);
        glLineWidth(2);
        glColor3f(0.4,0.0,0.0);


        glBegin(GL_LINE_LOOP);
        glVertex3f(0.0,0.0,0.0);
        for (int i(0); i < beamCount; i++)
            glVertex3f(ping.beam[i].sample[499].position[1][0],0.0,ping.beam[i].sample[499].position[1][1]);
        glVertex3f(ping.beam[beamCount-1].sample[499].position[2][0],0.0,ping.beam[beamCount-1].sample[499].position[2][1]);
        glEnd();

        if(showRaw)
        {
            for(int e = 0; e < 16; ++e)
            {
                glColor3f(1.0,1.0,1.0);
                glBegin(GL_LINE_STRIP);
                for(int s = 0; s < 500; ++s)
                {
                    glVertex3f(s*ping.range/500.0,0.0,5+e*2.0+ping.element[e][s]);
                }
                glEnd();

                glColor3f(1.0,0.0,0.0);
                glBegin(GL_LINE_STRIP);
                for(int s = 0; s < 500; ++s)
                {
                    glVertex3f(s*ping.range/500.0,0.0,5+e*2.0+ping.synt_element[e][s].real());
                }
                glEnd();

                glColor3f(0.0,1.0,0.0);
                glBegin(GL_LINE_STRIP);
                for(int s = 0; s < 500; ++s)
                {
                    glVertex3f(s*ping.range/500.0,0.0,5+e*2.0+ping.synt_element[e][s].imag());
                }
                glEnd();

//                 glColor3f(1.0,0.0,0.0);
//                 glBegin(GL_LINE_STRIP);
//                 for(unsigned int s = 0; s < ping.element_frequency[e].size(); ++s)
//                 {
//                     glVertex3f(s,0.0,e*2.0+ping.element_frequency[e][s].real()/10.0);
//                 }
//                 glEnd();
// 
//                 glColor3f(0.0,1.0,0.0);
//                 glBegin(GL_LINE_STRIP);
//                 for(unsigned int s = 0; s < ping.element_frequency[e].size(); ++s)
//                 {
//                     glVertex3f(s,0.0,e*2.0+ping.element_frequency[e][s].imag()/10.0);
//                 }
//                 glEnd();

                glColor3f(0.0,0.0,1.0);
                glBegin(GL_LINE_STRIP);
                for(unsigned int s = 0; s < ping.element_frequency[e].size(); ++s)
                {
                    glVertex3f(s*(37500.0/ping.range)/500.0,0.0,50+e*2.0+std::abs(ping.element_frequency[e][s])*50.0);
                }
                glEnd();

            }

            glPushMatrix();
            glColor3f(0.4,0.0,0.0);
            glTranslatef(ping.range,0.0,0.0);
            glBegin(GL_LINE_LOOP);
                glVertex3f(0.0,0.0,0.0);
                glVertex3f(0.0,0.0,-ping.range);
                glVertex3f(16.0*ping.sampleRange,0.0,-ping.range);
                glVertex3f(16.0*ping.sampleRange,0.0,0.0);
            glEnd();
            glPopMatrix();
        }

        if(showBeamformedRectangle)
        {
                glPushMatrix();
                glColor3f(0.4,0.0,0.0);
                glTranslatef(ping.range+60.0*ping.sampleRange,0.0,0.0);
                glBegin(GL_LINE_LOOP);
                    glVertex3f(0.0,0.0,0.0);
                    glVertex3f(0.0,0.0,-ping.range);
                    glVertex3f(beamCount*ping.sampleRange,0.0,-ping.range);
                    glVertex3f(beamCount*ping.sampleRange,0.0,0.0);
                glEnd();
                glPopMatrix();
        }

        for (int j = 0; j < 500-1; j++)
        {
            // beamformed (fan)
            
            glBegin(GL_QUADS);
            for (int i = 0; i < beamCount; i++)
            {
                float c = ping.beam[i].sample[j].amplitude;

                float r = 0.0;
                float g = 0.0;
                float b = 0.0;

                if(c < 0.2)
                {
                    b = c*5.0;
                }
                else if(c < 0.6)
                {
                    g = (c-0.2)*2.5;
                    b = 1.0-g;
                }
                else
                {
                    r = (c-0.6)*2.5;
                    g = 1.0-r;
                }

                glColor3f(r,g,b);
                for(int p(0); p<4; p++)
                    glVertex3f(ping.beam[i].sample[j].position[p][0],0.0,ping.beam[i].sample[j].position[p][1]);

            }
            glEnd();

            if (showRaw)
            {
                glPushMatrix();
                glTranslatef(ping.range,0.0,0.0);

                // raw data display

                glBegin(GL_QUADS);
                for (int e = 0; e < 16; e++)
                {
                    int i(e);
                    if (flipElements)
                        i = 15-e;

                    float c = ping.element[e][j];
                    if (c<0.0)
                        glColor3f(-c,0.0,0.0);
                    else
                        glColor3f(0.0,0.0,c);

                    glVertex3f((i+1)*ping.sampleRange,0.0,-(j)*ping.sampleRange);
                    glVertex3f((i+0)*ping.sampleRange,0.0,-(j)*ping.sampleRange);
                    glVertex3f((i+0)*ping.sampleRange,0.0,-(j+1.0)*ping.sampleRange);
                    glVertex3f((i+1)*ping.sampleRange,0.0,-(j+1.0)*ping.sampleRange);
                }
                glEnd();

                glPopMatrix();
            }

            if(showRawPhase)
            {
                glPushMatrix();
                glTranslatef(ping.range,0.0,0.0);
                for (int e = 0; e < 16; e++)
                {
                    int i = e;
                    if (flipElements)
                        i = 15-e;
                }
                glPopMatrix();
            }

            if (showBeamformedRectangle)
            {
                glPushMatrix();
                glTranslatef(ping.range+60.0*ping.sampleRange,0.0,0.0);

                // beamformed (rect)

                glBegin(GL_QUADS);
                for (int i = 0; i < beamCount; i++)
                {
                    float c = ping.beam[i].sample[j].amplitude;

                    float r = 0.0;
                    float g = 0.0;
                    float b = 0.0;

                    if(c < 0.2)
                    {
                        b = c*5.0;
                    }
                    else if(c < 0.6)
                    {
                        g = (c-0.2)*2.5;
                        b = 1.0-g;
                    }
                    else
                    {
                        r = (c-0.6)*2.5;
                        g = 1.0-r;
                    }

                    glColor3f(r,g,b);
                    glVertex3f(i*ping.sampleRange,0.0,-j*ping.sampleRange);
                    glVertex3f(i*ping.sampleRange,0.0,-(j+1)*ping.sampleRange);
                    glVertex3f((i+1)*ping.sampleRange,0.0,-(j+1)*ping.sampleRange);
                    glVertex3f((i+1)*ping.sampleRange,0.0,-j*ping.sampleRange);
                }
                glEnd();
                glPopMatrix();
            }

        }
        glEndList();
        needNewList = false;
    }
    if (displayList)
        glCallList(displayList);
    ObjectCollection::DrawLowRes( degradation,win);
}

int DeltaT::DrawHighRes(int eye, Window3D *win)
{
    DrawLowRes(0.0,win);
    return ObjectCollection::DrawHighRes(eye, win);
}

int DeltaT::Load()
{
    //check file type to know if we should load 837 or dt format
    int ret = Open();
    if (!ret)
    {
        if (fread(&header,sizeof(header),1,in) != 1 || header.hdr[0] != '8' || header.hdr[1] != '3' || header.hdr[2] != '7')
        {
            ret = 1;
        }
        else
        {
            data_id = header.id;
            recSize = swap(header.length);
            switch (data_id)
            {
                case DeltaT_iux:
                    fseek(in,sizeof(DeltaT_IUX_Data),SEEK_CUR);
                    std::cout << "iux " << recSize << std::endl;
                    std::cout << "header: " << sizeof(header) << " data: " << sizeof(DeltaT_IUX_Data) << std::endl;
                    break;
                case DeltaT_ivx:
                    fseek(in,sizeof(DeltaT_IVX_Data),SEEK_CUR);
                    std::cout << "ivx " << recSize << std::endl;
                    break;
                default:
                    ret = 1;
                    break;
            }
            if (!ret)
                if (!buildTimeIndex())
                    ret = 1;
        }
    }
    SetLabel( "deltat");
    return ret;
}

bool DeltaT::buildTimeIndex()
{
    int month_index = 13; // value outside 0-11 is considered null;
    TimePieces tp;

    bool done(false);
    while (!feof(in) && !done && header.hdr[0] == '8' && header.hdr[1] == '3' && header.hdr[2] == '7')
    {
        if(fread(&header,sizeof(header),1,in) == 0)
            done = true;
        std::string date_str((const char *)(header.date));
        int day =  atoi((const char *)(header.date));
        int year = atoi((const char *)&((header.date)[7]));
        std::string month(date_str.substr(3,3));
        //if we don't already have a month index within range or we have one but it is not for the month we are looking for...
        if ((month_index > 11 || month_index < 0) || (month_index <= 11 && month_index >= 0 && month_name[month_index] != month))
            for (month_index = 0; month_index < 12 && month_name[month_index] != month; month_index++);
        tp.setYearMonthDay( year,month_index,day);
        int hour = atoi((const char *)header.time);
        int minute = atoi((const char *)&(header.time[3]));
        int second = atoi((const char *)&(header.time[6]));
        tp.setHourMinuteSecond( hour,minute,second);
        double t(tp.compose());
        t+=atof((const char *)header.csec);
        times.push_back(t);
        bbox.expandTime( t);
        if (parent)
            parent->setBBoxStale();
        switch (data_id)
        {
            case DeltaT_iux:
                fseek(in,sizeof(DeltaT_IUX_Data),SEEK_CUR);
                break;
            case DeltaT_ivx:
                fseek(in,sizeof(DeltaT_IVX_Data),SEEK_CUR);
                break;
            default:
                break;
        }
    }
    return true;
}

void DeltaT::Idle(double time)
{
    if (!times.empty() && visible)
    {
        unsigned long oldIndex(timeIndex);
        if (times[0] >= time)
            timeIndex = 0;
        else if (times.back()<= time)
            timeIndex = times.size()-1;
        else
        {
            while (times[timeIndex] < time && timeIndex < times.size()-1 && times[timeIndex+1] <= time)
            {
                timeIndex++;
            }
            while (times[timeIndex] > time && timeIndex > 0 && times[timeIndex-1] > time)
            {
                timeIndex--;
            }
        }
        if (oldIndex != timeIndex)
            LoadPing(timeIndex);
    }
    ObjectCollection::Idle(time);
}

void DeltaT::LoadPing(unsigned long pingNo)
{
    fseek(in,pingNo*recSize,SEEK_SET);
    if (fread(&header,sizeof(header),1,in) != 1 ||header.hdr[0] != '8' || header.hdr[1] != '3' || header.hdr[2] != '7')
        return;
        switch (data_id)
        {
            case DeltaT_iux:
                //delete iux;
                if (!iux)
                    iux = new DeltaT_IUX_Data;
                if(fread(iux,sizeof(DeltaT_IUX_Data),1,in)!=1)
                    return;
                for(int e=0;e<16;++e)
                {
                    for(int s=0;s<500;++s)
                    {
                        ping.element[e][s] = (iux->data[s][e]-127.0)/128.0;
                    }
                    //dft.Transform(ping.element[e],ping.element_frequency[e]);
                    // freq domain hilbert transform
                    for(int i = 0; i < 500; ++i)
                        if(i>0 && i<250)
                                ping.element_frequency[e][i] *= 2.0;
                        else if (i > 250 && i < 500)
                                ping.element_frequency[e][i] = 0.0;
                    //dft.InverseTransform(ping.element_frequency[e],ping.synt_element[e]);
/*                    for(int s=0;s<500;++s)
                    {
                        double r = sqrt(std::abs(ping.synt_element[e][s].real()));
                        if (ping.synt_element[e][s].real() < 0.0)
                            r = -r;
                        double i = sqrt(std::abs(ping.synt_element[e][s].imag()));
                        if (ping.synt_element[e][s].imag() < 0.0)
                            i = -i;
                        ping.synt_element[e][s] = std::complex<double>(r,i);
                    }*/
                }

                beamform();
                break;
            case DeltaT_ivx:
                delete ivx;
                ivx = new DeltaT_IVX_Data;
                if(fread(ivx,sizeof(DeltaT_IVX_Data),1,in)!=1)
                    return;
                break;
            default:
                break;
        }
    globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
}

/// Pre calculate phase shifts when parameters change.
/// Such parameters are beam count, range, sound speed, frequency
/// swath angle or element spacing.
void DeltaT::adjustBeams()
{
    bool recalc(false);

    if(ping.beamCount != beamCount)
    {
        ping.beamCount = beamCount;
        recalc = true;
    }

    float soundSpeed(1500.0);
    if (header.sspeed&0x80)
        soundSpeed = (swap(header.sspeed)&0x7FFF)/10.0;
    if(ping.soundSpeed != soundSpeed)
    {
        ping.soundSpeed = soundSpeed;
        recalc = true;
    }

    float freq(swap(header.frequency)*1000.0);
    if(ping.frequency != freq)
    {
        ping.frequency = freq;
        recalc = true;
    }

    if (ping.range != iux->hdr.range)
    {
        ping.range = iux->hdr.range;
        recalc = true;
    }

    if (ping.swathAngle != swathAngle)
    {
        ping.swathAngle = swathAngle;
        recalc = true;
    }

    if (ping.elementSpacing != elementSpacing)
    {
        ping.elementSpacing = elementSpacing;
        recalc = true;
    }

    if (ping.flipElements != flipElements)
    {
        ping.flipElements = flipElements;
        recalc = true;
    }


    if(recalc)
    {
        std::cerr << "soundSpeed: " << soundSpeed << std::endl;
        ping.beam.resize(beamCount);
        ping.sampleRange = ping.range*ping.soundSpeed/(500.0*1500.0);
        ping.beamWidth = ping.swathAngle/beamCount;
        ping.startAngle = -ping.swathAngle/2.0;
        xOffset = -(ping.range * 2 + (100+2*beamCount)*ping.sampleRange);

/*        int flip(1);
        if (flipElements)
            flip = -1;*/
        for (int i = 0; i<beamCount; i++)
        {
            float a = -swathAngle/2.0+i*swathAngle/float(beamCount);
            for (int j = 0;j < 500; j++)
            {
                double d(j*ping.sampleRange);
/*                for(int e=0;e<16;e++)
                    ping.beam[i].sample[j].phaseShift[e] = (-7.5+e)*(M_PI/2.0 + sin(a)*elementSpacing/soundSpeed*freq*2.0*M_PI); *///(sqrt(pow((-7.5+e)*flip*elementSpacing-d*sin(a),2)+pow(d*cos(a),2))-d)/soundSpeed*freq*2.0*M_PI;

                double d2(d+ping.sampleRange);
                float a2(a+ping.beamWidth);

                ping.beam[i].sample[j].position[0][0] = sin(a)*d;
                ping.beam[i].sample[j].position[1][0] = sin(a)*d2;
                ping.beam[i].sample[j].position[2][0] = sin(a2)*d2;
                ping.beam[i].sample[j].position[3][0] = sin(a2)*d;

                ping.beam[i].sample[j].position[0][1] = cos(a)*-d;
                ping.beam[i].sample[j].position[1][1] = cos(a)*-d2;
                ping.beam[i].sample[j].position[2][1] = cos(a2)*-d2;
                ping.beam[i].sample[j].position[3][1] = cos(a2)*-d;
            }
        }
    }
}

/// Calculate amplitudes for each sample of each beam.
/// At each sample, apply phase shifts to raw element data
/// then sum Is and Qs, from which amplitude is calculated.
void DeltaT::beamform()
{
    adjustBeams();
    for (int j=0;j < 500; j++)
    {
        std::vector<std::complex<double> > elements;
        std::vector<std::complex<double> > beams;
        for(int e=0;e<16;e++)
        {
            elements.push_back(ping.synt_element[e][j]);//*(0.53836-0.46164*cos(2.0*M_PI*e/15.0)));
        }
        //bf_dft.Transform(elements,beams);
        for (int i = 0; i<beamCount; i++)
        {
            float a = -swathAngle/2.0+i*swathAngle/float(beamCount);
            int n = sin(a)*256/2;
            //ping.beam[i].sample[j].amplitude *= .75;
            ping.beam[i].sample[j].amplitude = std::abs(beams[(256-n)%256])*16.0;
        }
    }

/*    for (int i = 0; i<beamCount; i++)
    {
        double beamAngle = ping.startAngle+i*ping.beamWidth;
        double phaseShift = sin(beamAngle)*ping.elementSpacing/ping.soundSpeed*ping.frequency*2.0*M_PI;

        for (int j=0;j < 500; j++)
        {
            double I = 0.0;
            double Q = 0.0;
            double weight = 0;
            for(int e=0;e<16;e++)
            {
                //hamming window
                //double shade = 0.53836-0.46164*cos(2.0*M_PI*e/16.0);

                //hanning window
                double shade = 0.5-0.5*cos(2.0*M_PI*e/16.0);

                //double shade = 1.0;
                weight += shade;
                double amplitude = shade * std::abs(ping.synt_element[e][j]);
                double angle = std::arg(ping.synt_element[e][j])*frequencyRatio;

                double ps = phaseShift*(-8.0+e);
                I += cos(ps+angle) * amplitude;
                Q += sin(ps+angle) * amplitude;
            }
            ping.beam[i].sample[j].amplitude = sqrt(sqrt(pow(I,2)+pow(Q,2))/weight);
        }
    }*/
    needNewList = true;
}

int DeltaT::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    //LogState ls(&gzlog, "DeltaT::perform", TRACE);

    std::stringstream retString;
    if (argc)
    {
        std::string cmd(argv[0]);
        switch (argc)
        {
            case 1:
                if (cmd == "elementSpacing")
                {
                    retString << elementSpacing;
                }
                else if (cmd == "swathAngle")
                {
                    retString << swathAngle;
                }
                else if (cmd == "beamCount")
                {
                    retString << beamCount;
                }
                else if (cmd == "showRaw")
                {
                    retString << showRaw;
                }
                else if (cmd == "showRawPhase")
                {
                    retString << showRawPhase;
                }
                else if (cmd == "showRawBits")
                {
                    retString << showRawBits;
                }
                else if (cmd == "showBeamformedRectangle")
                {
                    retString << showBeamformedRectangle;
                }
                else if (cmd == "flipElements")
                {
                    retString << flipElements;
                }
                else if (cmd == "frequencyRatio")
                {
                    retString << frequencyRatio;
                }
                else
                    break;
                Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                return TCL_OK;
            case 2:
                //gzlog << argv[0] << " " << argv[1] << endl;
                if (cmd == "elementSpacing")
                {
                    elementSpacing = atof(argv[1]);
                }
                else if (cmd == "swathAngle")
                {
                    swathAngle = atof(argv[1]);
                }
                else if (cmd == "beamCount")
                {
                    beamCount = atoi(argv[1]);
                }
                else if (cmd == "showRaw")
                {
                    showRaw = isTrue(argv[1]);
                    needNewList = true;
                }
                else if (cmd == "showRawPhase")
                {
                    showRawPhase = isTrue(argv[1]);
                    needNewList = true;
                }
                else if (cmd == "showRawBits")
                {
                    showRawBits = isTrue(argv[1]);
                    needNewList = true;
                }
                else if (cmd == "flipElements")
                {
                    flipElements = isTrue(argv[1]);
                    needNewList = true;
                }
                else if (cmd == "frequencyRatio")
                {
                    frequencyRatio = atof(argv[1]);
                    needNewList = true;
                }
                else if (cmd == "showBeamformedRectangle")
                {
                    showBeamformedRectangle = isTrue(argv[1]);
                    needNewList = true;
                }
                else
                    break;
                beamform();
                globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
                return TCL_OK;
            default:
                break;
        }
    }
    return ObjectCollection::perform( argc, argv, interp);
}
