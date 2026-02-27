#include "IMU.h"
#include <gz4d/glo/Vertex.h>
#include "engine/InputMediator.h"
#include "engine/Scene3D.h"

REGISTER_OBJECT(IMU, "");

IMU::IMU()
    :dataQueue(imu.NewOutQueue()),mag_data(500),mag_raw(500),accel_data(500),accel_raw(500),gyro_data(500),gyro_raw(500),dindex(0),fade(1.0),showMag(true),showAccel(true),showGyro(true)
{
    imu.Open("00:06:66:00:D9:EE");
}

void IMU::drawLines(std::vector<gz4d::Vector<float,3> > const &v, float colorGain, float xoffset, float yoffset)
{
    for(int i = 0; i < 3; ++i)
    {
        glBegin(GL_LINE_STRIP);
        for (std::vector<gz4d::Vector<float,3> >::const_iterator d = v.begin(); d != v.end(); ++d)
        {
            std::size_t j = d-v.begin();
            float c;
            if (j < dindex)
                c = ((v.size()-dindex)+j)/float(v.size());
            else
                c = (j-dindex)/float(v.size());
            c *= colorGain;

            switch(i)
            {
                case 0:
                    glColor3f(1.0*c,0.0,0.0);
                    break;
                case 1:
                    glColor3f(0.0,1.0*c,0.0);
                    break;
                default:
                    glColor3f(0.0,0.0,1.0*c);
            }
            glVertex3f(xoffset+(d-v.begin())/float(v.size()),0.0,yoffset+(*d)[i]*.5);
        }
        glEnd();
    }
}

void IMU::DrawLowRes(float degradation, Window3D *win)
{
    glPushAttrib(GL_POINT_BIT|GL_LIGHTING_BIT);
    glPointSize(2.0);
    glColor3f(1.0,1.0,1.0);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    //glBegin(GL_POINTS);

    for (std::vector<gz4d::Vector<float,3> >::iterator d = mag_data.begin(); d != mag_data.end(); ++d)
    {
        std::size_t i = d-mag_data.begin();
        float c;
        if (i < dindex)
            c = ((mag_data.size()-dindex)+i)/float(mag_data.size());
        else
            c = (i-dindex)/float(mag_data.size());

        glColor3f(0.5*c,0.5*c,0.5*c);
        glVertex3f(0.0,0.0,0.0);
        glColor3f(1.0*c,1.0*c,1.0*c);
        gz4d::glo::Vertex(*d);
    }

    for (std::vector<gz4d::Vector<float,3> >::iterator d = accel_data.begin(); d != accel_data.end(); ++d)
    {
        std::size_t i = d-accel_data.begin();
        float c;
        if (i < dindex)
            c = ((accel_data.size()-dindex)+i)/float(accel_data.size());
        else
            c = (i-dindex)/float(accel_data.size());

        glColor3f(0.125*c,0.125*c,0.5*c);
        glVertex3f(0.0,0.0,0.0);
        glColor3f(0.25*c,0.25*c,1.0*c);
        gz4d::glo::Vertex(*d);
    }

    glEnd();

    drawLines(mag_data,1.0,-0.5,2.0);
    drawLines(mag_raw,0.5,-0.5,2.0);
    drawLines(accel_data,1.0,-1.6,2.0);
    drawLines(accel_raw,0.5,-1.6,2.0);
    drawLines(gyro_data,1.0,0.6,2.0);
    drawLines(gyro_raw,0.5,0.6,2.0);

    glPopAttrib();
}

void IMU::Idle(double time)
{
    while(!dataQueue->empty())
    {
        gz4d::IMU::RawState rs = dataQueue->pop();
        gz4d::IMU::State s = c(f(rs));
        gz4d::IMU::State rcs = c(rs);
        mag_data[dindex] = s.mag;
        accel_data[dindex] = s.accel;
        gyro_data[dindex] = s.gyro;
        mag_raw[dindex] = rcs.mag;
        accel_raw[dindex] = rcs.accel;
        gyro_raw[dindex] = rcs.gyro;

        ++dindex;
        if (dindex == mag_data.size())
            dindex = 0;
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
    
    Object::Idle( time);
}


int IMU::perform(int argc, const char** argv, Tcl_Interp* interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command ==  "kalman")
        {
            if(argc != 7)
            {
                Tcl_SetResult(interp, const_cast<char*>("usage: kalman mR mQ aR aQ gR gQ"), TCL_VOLATILE);
                return TCL_ERROR;
            }
            f.mR = boost::lexical_cast<float>(argv[1]);
            f.mQ = boost::lexical_cast<float>(argv[2]);
            f.aR = boost::lexical_cast<float>(argv[3]);
            f.aQ = boost::lexical_cast<float>(argv[4]);
            f.gR = boost::lexical_cast<float>(argv[5]);
            f.gQ = boost::lexical_cast<float>(argv[6]);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
    }
    return Object::perform(argc, argv, interp);
}

void IMU::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">IMU</h" << level << ">";

    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/kalman/mrValue/mqValue/arValue/aqValue/grValue/gqValue\">Kalman parameters:<br>"
            << "Mag R (measurement noise):<input type=\"text\" name=\"mrValue\" value=\"" << f.mR <<"\"/> "
            << "Mag Q (process  noise):<input type=\"text\" name=\"mqValue\" value=\"" << f.mQ <<"\"/><br>"
            << "Accel R (measurement noise):<input type=\"text\" name=\"arValue\" value=\"" << f.aR <<"\"/> "
            << "Accel Q (process  noise):<input type=\"text\" name=\"aqValue\" value=\"" << f.aQ <<"\"/><br>"
            << "Gyro R (measurement noise):<input type=\"text\" name=\"grValue\" value=\"" << f.gR <<"\"/> "
            << "Gyro Q (process  noise):<input type=\"text\" name=\"gqValue\" value=\"" << f.gQ <<"\"/><br>"
            << "<input type=\"submit\" value=\"Modify\"/></form><br>";

    Object::getHtmlInfo(html,level);
}

