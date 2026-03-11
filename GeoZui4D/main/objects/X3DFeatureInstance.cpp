// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include "X3DFeatureInstance.h"
#include "Feature.h"
#include <gz4d/X3D.h>
//#include <gz4d/util/Xml.h>
#include <iostream>
#include <gz4d/geo/Projection.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"

extern MainWindow *mainWindow;

X3DFeatureInstance::X3DFeatureInstance()
{
    feature = new Feature();
    typeName = "FeatureInstance";
}

void X3DFeatureInstance::LoadField( gz4d::SFString name, bpt::ptree const &x3d, gz4d::X3D* root)
{
    if (name == "priority")
    {
        float priority;
        LoadItem ( x3d, "value", priority );
        feature->setPriority(priority);
    }
    if (name == "minRange")
    {
        float m;
        LoadItem ( x3d, "value", m );
        feature->setMinRange(m);
    }
    if (name == "maxRange")
    {
        float m;
        LoadItem ( x3d, "value", m );
        feature->setMaxRange(m);
    }
    if (name == "emphasisFactor")
    {
        float m;
        LoadItem ( x3d, "value", m );
        feature->setEmphasisFactor(m);
    }
    else if (name == "scale")
    {
        gz4d::SFVec3f scale;
        LoadItem ( x3d, "value", scale );
        feature->setScale(scale);
    }
    else if (name == "viewTrackingRange")
    {
        gz4d::SFVec3f range;
        LoadItem ( x3d, "value", range );
        feature->setRange(range);
    }
    else if (name == "lineColor")
    {
        gz4d::SFColor color;
        LoadItem( x3d, "value", color );
        feature->setLineColor( color );
    }
    else if (name == "location_UTM" )
    {
        gz4d::SFVec3d location;
        LoadItem( x3d, "value", location );
        feature->setLocationUTM( location );
    }
    else if (name == "position")
    {
        std::string ps;
        enum {LAT, LON};
        gz4d::SFVec2d position;
        LoadItem( x3d, "value", position );
        ps = mainWindow->getScene3D()->getCurrentProjectionName();
        if (ps.empty())
        {
            gz4d::ogr::SpatialReference sr;
            sr.SetUTM(gz4d::geo::UTMZone(position[LON]), position[LAT] >= 0.0);
            mainWindow->getScene3D()->setSpatialReference(sr);
            ps = mainWindow->getScene3D()->getCurrentProjectionName();
        }
        gz4d::geo::Projection proj(ps);
        gz4d::Point<double> ret = proj(position[LAT], position[LON]);
        gz4d::SFVec3d location(ret[X], ret[Y], 0.0);
        feature->setLocationUTM(location);
    }
    else if (name == "children")
    {
        for(bpt::ptree::const_iterator node = x3d.begin(); node != x3d.end(); ++node)
        {
            gz4d::SFNode newnode = newNode(node->first, node->second, root);
            if (newnode)
            {
                children.push_back(newnode);
                feature->addChild(newnode);
            }
        }
    }
}

void X3DFeatureInstance::LoadStarted()
{
    feature->SetLabel(DEF);
}

void X3DFeatureInstance::LoadCompleted()
{
    feature->updateChildren();
}
