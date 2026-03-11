// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "OGR_Layer.h"
#include "OGR_Feature.h"
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include <gz4d/geo/Projection.h>

extern MainWindow * mainWindow;

REGISTER_OBJECT(OGR_Layer,"");

OGR_Layer::OGR_Layer(gz4d::ogr::Layer layer, OGR_Limits const &limits, float minSpan)
    :displayList(0)
{
    std::string name = layer.GetDefinition().GetName();
    if(!name.empty())
    {
        SetLabel(name);
    }

    if(layer.GetSpatialRef())
        spatialReference = layer.GetSpatialRef();

    double xref = 0.0;
    double yref = 0.0;


    gz4d::ogr::Envelope e = layer.GetExtent();
    if (e)
    {
        gz4d::Vector<double,2> center = gz4d::Box<gz4d::Vector<double,2> >(e).getCenter();
        autoProjection(center[0],center[1]);
        gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
        gz4d::Point<double> geo = proj(center[0],center[1]);
        SetPosition(geo[0],geo[1],0.0);
        xref = geo[0];
        yref = geo[1];
    }


    layer.ResetReading();
    gz4d::ogr::Feature f = layer.GetNextFeature();
    while(f)
    {
        OGR_Feature * o = new OGR_Feature(f,spatialReference,xref,yref,limits,minSpan);
        if(*o)
            addObject(o);
        else
            delete o;
        f = layer.GetNextFeature();
    }
}

void OGR_Layer::DrawLowRes(float degradation, Window3D *win)
{
    if(win->isRecording())
    {
        SimpleObjectCollection::DrawLowRes(0.0,win);
    }
    else
    {
        if(!displayList)
        {
            displayList = glGenLists(1);
            glNewList(displayList,GL_COMPILE);
            SimpleObjectCollection::DrawLowRes(0.0,win);
            glEndList();
        }
        glCallList(displayList);
    }
}
