// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "OGR_Polygon.h"
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/PushAttrib.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include <gz4d/geo/Projection.h>

extern MainWindow * mainWindow;

OGR_Polygon::OGR_Polygon(gz4d::ogr::Polygon polygon, gz4d::ogr::SpatialReference spatialRef, double xref, double yref, OGR_Limits const &limits, float minSpan)
{
    gz4d::ogr::SpatialReference sr = spatialRef;
    if(polygon.getSpatialReference())
        sr = polygon.getSpatialReference();

    gz4d::geo::Projection proj(sr,mainWindow->getScene3D()->getSpatialReference());

    gz4d::ogr::LinearRing ring = polygon.getExteriorRing();
    exteriorRingDir = ring.isClockwise();
 
    float minSpanSqr = minSpan*minSpan;

    gz4d::Point<double> lastp;
    
    
    for(gz4d::ogr::LinearRing::const_iterator p = ring.begin(); p != ring.end(); ++p)
    {
        if(limits(*p))
        {
            gz4d::Point<double> pp = proj(*p);
            bool skip = false;
            if(p != ring.begin() && p < ring.end() - 1)
            {
                float d = (pp[0]-lastp[0])*(pp[0]-lastp[0])+(pp[1]-lastp[1])*(pp[1]-lastp[1]);
                if (d < minSpanSqr)
                    skip = true;
            }
            if(!skip)
            {
                exteriorRing.push_back(gz4d::Point<float>(pp[0]-xref,pp[1]-yref,-(*p)[0]));
                lastp = pp;
            }
        }
    }

    interiorRings.resize(polygon.getNumInteriorRings());
    for(unsigned int i=0; i<interiorRings.size(); ++i)
    {
        lastp = gz4d::Point<double>();

        ring = polygon.getInteriorRing(i);
        interiorRingDirs.push_back(ring.isClockwise());

        for(gz4d::ogr::LinearRing::const_iterator p = ring.begin(); p != ring.end(); ++p)
        {
            if(limits(*p))
            {
                gz4d::Point<double> pp = proj(*p);
                bool skip = false;
                if(p != ring.begin() && p < ring.end() - 1)
                {
                    float d = (pp[0]-lastp[0])*(pp[0]-lastp[0])+(pp[1]-lastp[1])*(pp[1]-lastp[1]);
                    if (d < minSpanSqr)
                        skip = true;
                }
                if(!skip)
                {
                    interiorRings[i].push_back(gz4d::Point<float>(pp[0]-xref,pp[1]-yref,-(*p)[0]));
                    lastp = pp;
                }
            }
        }

    }
}

void OGR_Polygon::Draw(float degradation, Window3D *win)
{
    if(exteriorRing.empty())
        return;

    gz4d::glo::PushAttrib();
    glDisable(GL_LIGHTING);
    //if(!win->isRecording())
    {
        glEnable(GL_STENCIL_TEST);
    
        glClearStencil(0x0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glColorMask(false,false,false,false);
        glDepthMask(false);
        glStencilFunc(GL_ALWAYS,1,1);
        glStencilOp(GL_INVERT,GL_INVERT,GL_INVERT);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0,0.0,0.0);
        if(exteriorRingDir)
        {
            for(std::vector<gz4d::Point<float> >::reverse_iterator p = exteriorRing.rbegin(); p != exteriorRing.rend(); ++p)
            {
                gz4d::glo::Vertex(*p);
            }
            gz4d::glo::Vertex(*exteriorRing.rbegin());
        }
        else
        {
            for(std::vector<gz4d::Point<float> >::iterator p = exteriorRing.begin(); p != exteriorRing.end(); ++p)
            {
                gz4d::glo::Vertex(*p);
            }
            gz4d::glo::Vertex(*exteriorRing.rbegin());
        }
        glEnd();
    
        for(unsigned int i = 0; i < interiorRings.size(); ++i)
        {
            if(!interiorRings[i].empty())
            {
                glBegin(GL_TRIANGLE_FAN);
                glVertex3f(0.0,0.0,0.0);
                if(interiorRingDirs[i])
                {
                    for(std::vector<gz4d::Point<float> >::reverse_iterator p = interiorRings[i].rbegin(); p != interiorRings[i].rend(); ++p)
                    {
                        gz4d::glo::Vertex(*p);
                    }
                    gz4d::glo::Vertex(*interiorRings[i].rbegin());
                }
                else
                {
                    for(std::vector<gz4d::Point<float> >::iterator p = interiorRings[i].begin(); p != interiorRings[i].end(); ++p)
                    {
                        gz4d::glo::Vertex(*p);
                    }
                    gz4d::glo::Vertex(*interiorRings[i].rbegin());
                }
                glEnd();
            }
        }
        
        glColorMask(true,true,true,true);
        glDepthMask(true);
        glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc (GL_NOTEQUAL, 0, 0x1);
    
        glColor3f(0.8,0.8,0.8);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0,0.0,0.0);
        if(exteriorRingDir)
        {
            for(std::vector<gz4d::Point<float> >::reverse_iterator p = exteriorRing.rbegin(); p != exteriorRing.rend(); ++p)
            {
                gz4d::glo::Vertex(*p);
            }
            gz4d::glo::Vertex(*exteriorRing.rbegin());
        }
        else
        {
            for(std::vector<gz4d::Point<float> >::iterator p = exteriorRing.begin(); p != exteriorRing.end(); ++p)
            {
                gz4d::glo::Vertex(*p);
            }
            gz4d::glo::Vertex(*exteriorRing.begin());
        }
        glEnd();
    
        for(unsigned int i = 0; i < interiorRings.size(); ++i)
        {
            if(!interiorRings[i].empty())
            {
                glBegin(GL_TRIANGLE_FAN);
                glVertex3f(0.0,0.0,0.0);
                if(interiorRingDirs[i])
                {
                    for(std::vector<gz4d::Point<float> >::reverse_iterator p = interiorRings[i].rbegin(); p != interiorRings[i].rend(); ++p)
                    {
                        gz4d::glo::Vertex(*p);
                    }
                    gz4d::glo::Vertex(*interiorRings[i].rbegin());
                }
                else
                {
                    for(std::vector<gz4d::Point<float> >::iterator p = interiorRings[i].begin(); p != interiorRings[i].end(); ++p)
                    {
                        gz4d::glo::Vertex(*p);
                    }
                    gz4d::glo::Vertex(*interiorRings[i].rbegin());
                }
                glEnd();
            }
        }
    
        glDisable(GL_STENCIL_TEST);
    }

    glColor3f(0.9,0.9,0.9);
    glDepthFunc(GL_LEQUAL);
    glBegin(GL_LINE_LOOP);
    if(exteriorRingDir)
    {
        for(std::vector<gz4d::Point<float> >::reverse_iterator p = exteriorRing.rbegin(); p != exteriorRing.rend(); ++p)
        {
            gz4d::glo::Vertex(*p);
        }
    }
    else
    {
        for(std::vector<gz4d::Point<float> >::iterator p = exteriorRing.begin(); p != exteriorRing.end(); ++p)
        {
            gz4d::glo::Vertex(*p);
        }
    }
    glEnd();

    for(unsigned int i = 0; i < interiorRings.size(); ++i)
    {
        if(!interiorRings[i].empty())
        {
            glBegin(GL_LINE_LOOP);
            if(interiorRingDirs[i])
            {
                for(std::vector<gz4d::Point<float> >::reverse_iterator p = interiorRings[i].rbegin(); p != interiorRings[i].rend(); ++p)
                {
                    gz4d::glo::Vertex(*p);
                }
            }
            else
            {
                for(std::vector<gz4d::Point<float> >::iterator p = interiorRings[i].begin(); p != interiorRings[i].end(); ++p)
                {
                    gz4d::glo::Vertex(*p);
                }
            }
            glEnd();
        }
    }

}
