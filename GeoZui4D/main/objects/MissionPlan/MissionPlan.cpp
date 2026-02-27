/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "GZincludes.h"
#include <sstream>
#include <iomanip>

#include "objects/MissionPlan/MissionPlan.h"
#include "MPCommand.h"
#include "engine/ViewManager.h"
#include "engine/PosableStateBase.h"

extern MainWindow * mainWindow;

REGISTER_OBJECT(MissionPlan, "");

MissionPlan::MissionPlan()
{
    selectedCmd = NULL;
    allowBaseExternalEdit = true;
    isEditableFlag = true;
//    socket.SetNonBlocking();
}

MissionPlan::~MissionPlan()
{
}

int MissionPlan::DrawHighRes(int eye, Window3D *win)
{
    
    DrawLowRes(0.0,win);
    return 1;
}

void MissionPlan::DrawLowRes(float degradation, Window3D *win)
{
    drawGeoRefPoint();
    
    //glPushName(0);
    glPushMatrix();
    glLoadIdentity();
    ObjectCollection::DrawLowRes(degradation,win);
    glPopMatrix();
    //glPopName();
}


void MissionPlan::drawGeoRefPoint()
{
    glPushAttrib(GL_POINT_BIT|GL_LIGHTING_BIT|GL_LINE_BIT);
    //glLoadName(0);
    //glPushName(1);
    glColor3f(0.9,0.9,0.9);
    glDisable(GL_LIGHTING);
    glPointSize(6);
    glBegin(GL_POINTS);
    glVertex3d(0.0,0.0,0.0);
    glEnd();
    glutSolidSphere(0.065,10,10);
    glLineWidth(4);
    glBegin(GL_LINES);
        glVertex3d(0.0,1.0,0.0);
        glVertex3d(0.0,-1.0,0.0);
        glVertex3d(0.75,0.0,0.0);
        glVertex3d(-0.75,0.0,0.0);
        glVertex3d(0.0,1.0,0.0);
        glVertex3d(-0.1,0.85,0.0);
        glVertex3d(0.0,1.0,0.0);
        glVertex3d(0.1,0.85,0.0);
    glEnd();
    glPopAttrib();
}

void MissionPlan::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
//     if (socket)
//     {
//         if (pose == whichPose)
//         {
//             const double* p = pose->getPosition();
//             std::stringstream s;
//             s << "<mp><setgeoref><position x=\"" << std::fixed << std::setw(12) << p[0] << "\" y=\"" << std::setw(12) << p[1] << "\" z=\"" << p[2] << "\"/>";
//             if (proj)
//             {
//                 double lon,lat;
//                 proj->makeCurrent();
//                 proj->invert(p[0], p[1], lon, lat);
//                 s << "<geographic longitude=\"" << lon << "\" latitude=\"" << lat << "\"/>";
//             }
//             s << "</setgeoref></mp>";
//             socket.Send(s.str());
//         }
//     }

    ObjectCollection::PoseChanged(whichPose, whatChanged);
}

void MissionPlan::getHtmlInfo(std::stringstream & html, short level)
{
    ObjectCollection::getHtmlInfo(html,level);
    
    html << "<h" << level << ">MissionPlan:</h" << level << ">";
    
    html << "Connection:";
    gz4d::IPSocket::Address a;
/*    try
    {
        socket.GetLocalAddress(a);
        html << " local: " << a.GetName() << ":" << a.GetPort();
        socket.GetRemoteAddress(a);
        html << " remote: " << a.GetName() << ":" << a.GetPort();
    }
    catch(gz4d::Socket::Exception e)
    {
        html << " (socket error) " << e.what();
    }*/
    html << "<BR>";
}

void MissionPlan::Idle(double time)
{
    std::string packet(1500,0);
    gz4d::IPSocket::Address a;
    try
    {
//         socket.RecvFrom(packet,a);
//         gzlog << "packet received: " << packet << endl;

/*        gz4d::Xml cmd(packet);
        if (cmd.hasDoc() && cmd.docName() == "mp")
        {
            cmd.childNode();
            while(cmd.hasCurrentNode())
            {
                //gzlog << "node: " << cmd.nodeName() << endl;
                int id;
                bool hasID = cmd.getInt(id,"id");
                if(hasID)
                {
                    if(cmd.nodeName() == "newcmd")
                    {
                        commands[id] = MPCommand(id,cmd);
                        AddObject(&commands[id]);
                    }
                    else if (commands.find(id) != commands.end())
                    {
                            commands[id].command(cmd);
                    }
                }
                else
                {
                    if (cmd.nodeName() == "getpos")
                    {
                            sendPosition(id);
                    }
                    else if(cmd.nodeName() == "setgeoref")
                    {
                        int zone;
                        if (cmd.getInt(zone,"zone"))
                        {
                            mainWindow->getScene3D()->setProjection(UTMzones[zone]);
                            setUTMProjection(zone);
                        }
                        cmd.childNode();
                        while(cmd.hasCurrentNode())
                        {
                            if(cmd.nodeName() == "position")
                            {
                                Point3d p;
                                cmd.getDouble(p[0],"x");
                                cmd.getDouble(p[1],"y");
                                cmd.getDouble(p[2],"z");
                                SetPosition(p);
                            }
                            cmd.nextNode();
                        }
                        cmd.parentNode();
                    }
                    else if(cmd.nodeName() == "setview")
                    {
                        cmd.childNode();
                        while(cmd.hasCurrentNode())
                        {
                            if(cmd.nodeName() == "position")
                            {
                                Point3d p;
                                cmd.getDouble(p[0],"x");
                                cmd.getDouble(p[1],"y");
                                cmd.getDouble(p[2],"z");
                                mainWindow->getViewInfo()->getPosable()->setPosition(p);
                            }
                            cmd.nextNode();
                        }
                        cmd.parentNode();
                    }
                }
                cmd.nextNode();
            }
        }*/
    }
    catch(gz4d::Socket::WouldBlockException){}

    ObjectCollection::Idle(time);
}

void MissionPlan::sendToGUI(std::string msg)
{
    //socket.Send(msg);
}


void MissionPlan::sendPosition(int id)
{
    std::stringstream retcmd;
    retcmd << "<mp><update id=\"" << id << "\"><position x=\"";
    retcmd << std::fixed << mainWindow->getViewInfo()->getPosable()->getPosition()[X] << "\" y=\"";
    retcmd << mainWindow->getViewInfo()->getPosable()->getPosition()[Y] << "\" z=\"";
    retcmd << mainWindow->getViewInfo()->getPosable()->getPosition()[Z] << "\"/></update></mp>";
    sendToGUI(retcmd.str());
}

void MissionPlan::cmdSelected(MPCommand * newSelected)
{
    if (selectedCmd && selectedCmd != newSelected)
        selectedCmd->itemSelected(0, false);
    selectedCmd = newSelected;
}

int MissionPlan::pickAction (Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
    if(pickType == PICK_DOUBLE1)
        return Object::pickAction(PICK_CLICK1, inputInfo, names, hits);
    return Object::pickAction(pickType, inputInfo, names, hits);
}
