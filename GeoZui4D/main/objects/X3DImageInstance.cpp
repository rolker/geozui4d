/* Copyright (c) 2008 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include "X3DImageInstance.h"
#include "TextureScreen.h"
#include <gz4d/X3D.h>
//#include <gz4d/util/Xml.h>
#include <iostream>

X3DImageInstance::X3DImageInstance()
{
    image = new TextureScreen();
    image->setWidth(0.0);
    image->setHeight(0.0);
    image->setColorDepth(0, false);
    typeName = "ImageInstance";
}

void X3DImageInstance::LoadField( gz4d::SFString name, bpt::ptree const &x3d, gz4d::X3D* root)
{
    if (name == "height")
    {
        float height;
        LoadItem ( x3d, "value", height );
        image->setHeight(height);
    }
    else if (name == "width")
    {
        float width;
        LoadItem ( x3d, "value", width );
        image->setWidth(width);
    }
    else if (name == "yaw")
    {
        float yaw;
        LoadItem ( x3d, "value", yaw );
        image->setYaw(yaw);
    }
    else if (name == "pitch")
    {
        float pitch;
        LoadItem ( x3d, "value", pitch );
        image->setPitch(pitch);
    }
    else if (name == "roll")
    {
        float roll;
        LoadItem ( x3d, "value", roll );
        image->setRoll(roll);
    }
    else if (name == "quality")
    {
        float quality;
        LoadItem ( x3d, "value", quality );
        image->overrideQuality(quality);
    }
    else if (name == "scaleFactor")
    {
        float scaleFactor;
        LoadItem ( x3d, "value", scaleFactor );
        image->setScale(scaleFactor);
    }/*
    else if (name == "rotation")
    {
        gz4d::SFRotation rotation;
        LoadItem ( x3d, "value", rotation );
        image->setRotation(rotation);
    }*/
    else if (name == "lineColor")
    {
        gz4d::SFColor color;
        LoadItem( x3d, "value", color );
        image->setLineColor( &color[0] );
    }
    else if (name == "location_offset" || name == "position") //"position" should really translate to lat/lon...?
    {
        gz4d::SFVec3d location;
        LoadItem( x3d, "value", location );
        image->setLocationOffset( location );
    }
    else if (name == "url")
    {
        gz4d::MFString url;
        LoadArray( x3d, "value", url);

        //std::string fn(url[0].substr(1,url[0].size()-2));
        image->loadTexture(bfs::path(root->getFilename()).parent_path()/url[0]); //&&&we need to loop through until we find one, rather than only try first one
        //image->buildTexture();
    }
}

void X3DImageInstance::LoadCompleted()
{
    image->SetLabel(DEF);
    image->applyChanges();
}
