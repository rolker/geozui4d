/* Copyright (c) 2010 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"TimeVaryingPoints.h"
//#include <gz4d/util/Xml.h>
#include <gz4d/geo/Projection.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include <gz4d/glo/Vertex.h>
#include <gz4d/vector/ios.h>
#include <gz4d/glo/Translate.h>
#include "engine/TextureMaps/TextureMap.h"

REGISTER_OBJECT(TimeVaryingPoints, "");

extern MainWindow *mainWindow;


TimeVaryingPoints::TimeVaryingPoints()
:basePoint(gz4d::Nan<double>(),gz4d::Nan<double>(),gz4d::Nan<double>()),fadeStart(3600.0*24),fadeEnd(3600.0*48),sizeScale(1.0),sizeMin(1.0),sizeMax(1000.0)
{
    DefineAttribute("fadeStart","Fade start","Time in seconds after point's timestamp to start fading",fadeStart);
    DefineAttribute("fadeEnd","Fade end","Time in seconds after point's timestamp to end fading",fadeEnd);
    DefineAttribute("sizeScale","Size scale","Scale factor applied to values to determine radius in meters",sizeScale);
    DefineAttribute("sizeMin","Minimum size","Minimum size in meters",sizeMin);
    DefineAttribute("sizeMax","Maximum size","Maximum size in meters",sizeMax);
}


void TimeVaryingPoints::DrawLowRes(float degradation, std::shared_ptr<Window3D> win)
{
    std::shared_ptr<TextureMap> currentTexture;
    if (!customtextureptr)
        currentTexture = *defaulttextureptr;
    else
        currentTexture = customtextureptr;

    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);

    EnableTexture(TEXTURE_COORD_NONE);

    glColor3f(1.0,1.0,1.0);
    glPointSize(3.0);
    glBegin(GL_POINTS);
    for(std::vector<Point>::iterator i = points.begin(); i != points.end(); ++i)
    {
        if(lastIdleTime >= i->values.begin()->first && lastIdleTime <= i->values.rbegin()->first+fadeEnd)
        {
            if (fadeEnd>fadeStart && lastIdleTime > i->values.begin()->first + fadeStart)
            {
                float c = 1.0-(lastIdleTime - (i->values.begin()->first+fadeStart))/(fadeEnd-fadeStart);
                glColor3f(c,c,c);
            }
            else
                glColor3f(1.0,1.0,1.0);
            currentTexture->SetHeight(i->values.begin()->second.value);
            gz4d::glo::Vertex(i->values.begin()->second.offset);
        }
    }
    glEnd();

    glEnable(GL_LIGHTING);

    for(std::vector<Point>::iterator i = points.begin(); i != points.end(); ++i)
    {
        if(lastIdleTime >= i->values.begin()->first && lastIdleTime <= i->values.rbegin()->first+fadeEnd)
        {
            if (fadeEnd>fadeStart && lastIdleTime > i->values.begin()->first + fadeStart)
            {
                float c = 1.0-(lastIdleTime - (i->values.begin()->first+fadeStart))/(fadeEnd-fadeStart);
                glColor3f(c,c,c);
            }
            else
                glColor3f(1.0,1.0,1.0);

            currentTexture->SetHeight(i->values.begin()->second.value);
            glPushMatrix();
            gz4d::glo::Translate(i->values.begin()->second.offset);
            float v = i->values.begin()->second.value*sizeScale;
            v = std::max(v,sizeMin);
            v = std::min(v,sizeMax);
            glScalef(v,v,v);
            if (!displayList.Call())
            {
                for(int r = 0; r < 16; ++r)
                {
                    glBegin(GL_QUAD_STRIP);
                    for(int c = 0; c <= 16; ++c)
                    {
                        double r1 = r*0.196349541; // pi/16
                        double r2 = (r+1)*0.196349541;
                        double c1 = c*0.392699082; // 2pi/16
                        double cr1 = cos(r1);
                        double sr1 = sin(r1);
                        double cr2 = cos(r2);
                        double sr2 = sin(r2);
                        double cc1 = cos(c1);
                        double sc1 = sin(c1);

                        glNormal3d(sr2*cc1,sr2*sc1,cr2);
                        glVertex3d(sr2*cc1,sr2*sc1,cr2);

                        glNormal3d(sr1*cc1,sr1*sc1,cr1);
                        glVertex3d(sr1*cc1,sr1*sc1,cr1);
                    }
                    glEnd();
                }
                displayList.End();
            }
            glPopMatrix();
        }
    }
    DisableTexture();
}

void TimeVaryingPoints::Idle(gz4d::Time time)
{
    if(time != lastIdleTime)
    {
        if((time >= bbox.start && time <= bbox.end + fadeEnd) || (lastIdleTime >= bbox.start && lastIdleTime <= bbox.end + fadeEnd))
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
    Object::Idle(time);
}


Object * TimeVaryingPoints::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    gz4d::geo::Projection proj;

    Object::LoadXML(xml,path, ncfile);

    for(gz4d::xml::Node n = xml.GetFirstNode("Point"); n; n.Next("Point"))
    {
        Point p;
        for(gz4d::xml::Node vn = n.GetFirstNode("Value"); vn; vn.Next("Value"))
        {
            Value v;
            gz4d::Time t;
            gz4d::xml::Attribute a = vn.GetFirstAttribute("position");
            if(a)
            {
                std::stringstream ss(a.Value());
                gz4d::Point<double> pos;
                ss >> pos[0] >> pos[1] >> pos[2];
                autoProjection(pos[X],pos[Y]);
                if(!proj)
                    proj = gz4d::geo::Projection(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                pos = proj(pos);
                if(gz4d::IsNan(basePoint[0]))
                {
                    basePoint = pos;
                    SetPosition(basePoint);
                    std::cerr << "base point:  " << basePoint << std::endl;
                }
                bbox.expand(pos);
                v.offset = pos - basePoint;
                //std::cerr << v.offset << std::endl;
            }
            a = vn.GetFirstAttribute("value");
            if(a)
                v.value = a.Value<double>();
            a = vn.GetFirstAttribute("time");
            if(a)
                t = gz4d::Time(a.Value<double>());
            a = vn.GetFirstAttribute("age");
            if(a)
                v.age = a.Value<double>();
            if(t)
            {
                p.values[t] = v;
                expandBBoxTime(t);
            }
        }
        if(!p.values.empty())
            points.push_back(p);
    }

    return this;
}

TimeVaryingPoints::Value::Value()
    :value(0.0),age(0.0)
{
}
