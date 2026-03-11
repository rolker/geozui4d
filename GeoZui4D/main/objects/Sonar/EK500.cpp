// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "EK500.h"
#include "engine/InputMediator.h"
#include "engine/Scene3D.h"
#include "MainWindow.h"
#include "engine/ObjectPoseState.h"
#include <gz4d/util/Utilities.h>
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/glo/Vertex.h>

extern MainWindow * mainWindow;

REGISTER_OBJECT(EK500, "");

EK500::EK500()
{
    baseTime = gz4d::Nan<double>();
    basePosition[0] = 0.0;
    basePosition[1] = 0.0;
    basePosition[2] = 0.0;
    zones.push_back(TimeZone());
}

void EK500::DrawLowRes(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT);
    glPointSize(5);
    glColor3f(1.0,1.0,1.0);
    glBegin(GL_POINTS);

    float startTime = 0.0;
    float endTime = 0.0;
    for(std::vector<TimeZone>::iterator i = zones.begin(); i != zones.end(); i++)
    {
        if (i->start.time < startTime)
            startTime = i->start.time;
        if (i->end.time > endTime)
            endTime = i->end.time;
    }

    float localIdleTime = lastIdleTime - baseTime;
    for(std::vector<Target>::iterator i = targets.begin(); i != targets.end(); i++)
    {
        if(localIdleTime >= i->time+startTime && localIdleTime <= i->time+endTime)
        {
            gz4d::glo::Vertex(i->position);
        }
    }
    glEnd();
}

int EK500::perform(int argc, const char** argv, Tcl_Interp* interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command ==  "addTarget")
        {
            if(argc == 6)
            {
                double time = atof(argv[1]);
                double x = atof(argv[2]);
                double y = atof(argv[3]);
                if(gz4d::IsNan(baseTime))
                {
                    baseTime = time;
                    basePosition[0] = x;
                    basePosition[1] = y;
                    SetPosition(x,y,0.0);
                }
                Target t;
                t.time = time-baseTime;
                t.position[0] = x-basePosition[0];
                t.position[1] = y-basePosition[1];
                t.position[2] = atof(argv[4]);
                t.value = atof(argv[5]);
                targets.push_back(t);
                expandBBoxTime(time);
                bbox.expand(gz4d::Point<double>(t.position));
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                return TCL_OK;
            }
            Tcl_SetResult(interp, const_cast<char*>("usage: addTarget time x y z value"), TCL_VOLATILE);
            return TCL_ERROR;
        }
        if (command == "zone")
        {
            if (argc == 2)
            {
                std::string subcmd(argv[1]);
                if (subcmd == "add")
                {
                    zones.push_back(TimeZone());
                    std::stringstream ret;
                    ret << zones.size()-1;
                    Tcl_SetResult(interp, (char *)(ret.str().c_str()), TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            if (argc > 2)
            {
                int znum = atoi(argv[1]);
                if (znum >= 0 && znum < (int)(zones.size()))
                {
                    if (argc >= 3)
                    {
                        std::string subcmd(argv[2]);
                        if(subcmd == "startTime")
                        {
                            std::stringstream ret;
                            switch (argc)
                            {
                                case 4:
                                    zones[znum].start.time = atof(argv[3]);
                                case 3:
                                    ret << zones[znum].start.time;
                                    Tcl_SetResult(interp, (char *)(ret.str().c_str()), TCL_VOLATILE);
                                    return TCL_OK;
                                default:
                                    Tcl_SetResult(interp, const_cast<char*>("usage: startTime [newTime]"), TCL_VOLATILE);
                                    return TCL_ERROR;
                            }
                        }
                        if(subcmd == "endTime")
                        {
                            std::stringstream ret;
                            switch (argc)
                            {
                                case 4:
                                    zones[znum].end.time = atof(argv[3]);
                                case 3:
                                    ret << zones[znum].end.time;
                                    Tcl_SetResult(interp, (char *)(ret.str().c_str()), TCL_VOLATILE);
                                    return TCL_OK;
                                default:
                                    Tcl_SetResult(interp, const_cast<char*>("usage: endTime [newTime]"), TCL_VOLATILE);
                                    return TCL_ERROR;
                            }
                        }
                        if(subcmd == "startSize")
                        {
                            std::stringstream ret;
                            switch (argc)
                            {
                                case 4:
                                    zones[znum].start.size = atof(argv[3]);
                                case 3:
                                    ret << zones[znum].start.size;
                                    Tcl_SetResult(interp, (char *)(ret.str().c_str()), TCL_VOLATILE);
                                    return TCL_OK;
                                default:
                                    Tcl_SetResult(interp, const_cast<char*>("usage: startSize [newSize]"), TCL_VOLATILE);
                                    return TCL_ERROR;
                            }
                        }
                        if(subcmd == "endSize")
                        {
                            std::stringstream ret;
                            switch (argc)
                            {
                                case 4:
                                    zones[znum].end.size = atof(argv[3]);
                                case 3:
                                    ret << zones[znum].end.size;
                                    Tcl_SetResult(interp, (char *)(ret.str().c_str()), TCL_VOLATILE);
                                    return TCL_OK;
                                default:
                                    Tcl_SetResult(interp, const_cast<char*>("usage: endSize [newSize]"), TCL_VOLATILE);
                                    return TCL_ERROR;
                            }
                        }
                        if(subcmd == "startColor")
                        {
                            std::stringstream ret;
                            switch (argc)
                            {
                                case 7:
                                    zones[znum].start.color[0] = atof(argv[3]);
                                    zones[znum].start.color[1] = atof(argv[4]);
                                    zones[znum].start.color[2] = atof(argv[5]);
                                    zones[znum].start.color[3] = atof(argv[6]);
                                case 3:
                                    ret << zones[znum].start.color[0] << " " << zones[znum].start.color[1] << " " << zones[znum].start.color[2] << " " << zones[znum].start.color[3];
                                    Tcl_SetResult(interp, (char *)(ret.str().c_str()), TCL_VOLATILE);
                                    return TCL_OK;
                                default:
                                    Tcl_SetResult(interp, const_cast<char*>("usage: startColor [r g b a]"), TCL_VOLATILE);
                                    return TCL_ERROR;
                            }
                        }
                        if(subcmd == "endColor")
                        {
                            std::stringstream ret;
                            switch (argc)
                            {
                                case 7:
                                    zones[znum].end.color[0] = atof(argv[3]);
                                    zones[znum].end.color[1] = atof(argv[4]);
                                    zones[znum].end.color[2] = atof(argv[5]);
                                    zones[znum].end.color[3] = atof(argv[6]);
                                case 3:
                                    ret << zones[znum].end.color[0] << " " << zones[znum].end.color[1] << " " << zones[znum].end.color[2] << " " << zones[znum].end.color[3];
                                    Tcl_SetResult(interp, (char *)(ret.str().c_str()), TCL_VOLATILE);
                                    return TCL_OK;
                                default:
                                    Tcl_SetResult(interp, const_cast<char*>("usage: endColor [r g b a]"), TCL_VOLATILE);
                                    return TCL_ERROR;
                            }
                        }
                    }
                }
                Tcl_SetResult(interp, const_cast<char*>("Invalid zone number"), TCL_VOLATILE);
                return TCL_ERROR;
            }
            Tcl_SetResult(interp, const_cast<char*>("zone add | id [startTime|endTime|startColor|endColor|startSize|endSize]"), TCL_VOLATILE);
            return TCL_ERROR;
        }
        return Object::perform(argc,argv,interp);
    }
    Tcl_SetResult(interp, const_cast<char*>("Valid commands:\n\taddTarget\t-Add a target"), TCL_VOLATILE);
    return TCL_ERROR;

}

void EK500::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">EK500 Channel:</h" << level << ">";
    html << "Target count: " << targets.size() << "<br>";
    html << "<table><tr>";
    html << "<th>Zone</th>";
    html << "<th>Start</th>";
    html << "<th>Finish</th></tr>";
    for (unsigned int i = 0; i < zones.size(); i++)
    {
        html << "<tr><td>" << i << "</td>";

        html << "<td>";
        html << "<form action=\"/command/gz/tellObject/" << GetId() << "/zone/" << i << "/startTime/sTime\">Time<input type=\"text\" name=\"sTime\" value=\"" << zones[i].start.time <<"\"/><input type=\"submit\" value=\"Set\"/></form>";
        html << "<form action=\"/command/gz/tellObject/" << GetId() << "/zone/" << i << "/startSize/sSize\">Size<input type=\"text\" name=\"sSize\" value=\"" << zones[i].start.size <<"\"/><input type=\"submit\" value=\"Set\"/></form>";
        html << "<form action=\"/command/gz/tellObject/" << GetId() << "/zone/" << i << "/startColor/sRed/sGreen/sBlue/sAlpha\">Color ";
        html << "r<input type=\"text\" name=\"sRed\" value=\"" << zones[i].start.color[0] <<"\"/>";
        html << "g<input type=\"text\" name=\"sGreen\" value=\"" << zones[i].start.color[1] <<"\"/>";
        html << "b<input type=\"text\" name=\"sBlue\" value=\"" << zones[i].start.color[2] <<"\"/>";
        html << "a<input type=\"text\" name=\"sAlpha\" value=\"" << zones[i].start.color[3] <<"\"/>";
        html << "<input type=\"submit\" value=\"Set\"/></form>";
        html << "</td>";

        html << "<td>";
        html << "<form action=\"/command/gz/tellObject/" << GetId() << "/zone/" << i << "/endTime/eTime\">Time<input type=\"text\" name=\"eTime\" value=\"" << zones[i].end.time <<"\"/><input type=\"submit\" value=\"Set\"/></form>";
        html << "<form action=\"/command/gz/tellObject/" << GetId() << "/zone/" << i << "/endSize/eSize\">Size<input type=\"text\" name=\"eSize\" value=\"" << zones[i].end.size <<"\"/><input type=\"submit\" value=\"Set\"/></form>";
        html << "<form action=\"/command/gz/tellObject/" << GetId() << "/zone/" << i << "/endColor/eRed/eGreen/eBlue/eAlpha\">Color ";
        html << "r<input type=\"text\" name=\"eRed\" value=\"" << zones[i].end.color[0] <<"\"/>";
        html << "g<input type=\"text\" name=\"eGreen\" value=\"" << zones[i].end.color[1] <<"\"/>";
        html << "b<input type=\"text\" name=\"eBlue\" value=\"" << zones[i].end.color[2] <<"\"/>";
        html << "a<input type=\"text\" name=\"eAlpha\" value=\"" << zones[i].end.color[3] <<"\"/>";
        html << "<input type=\"submit\" value=\"Set\"/></form>";
        html << "</td>";
        html << "</tr>";
    }
    html << "</table>";

    Object::getHtmlInfo(html,level);
}


EK500::Attributes::Attributes()
{
    time = 0.0;
    color[0]=1.0;
    color[1]=1.0;
    color[2]=1.0;
    color[3]=1.0;
    size = 0.1;
}

EK500::TimeZone::TimeZone()
{
    end.time = 3600.0;
}


