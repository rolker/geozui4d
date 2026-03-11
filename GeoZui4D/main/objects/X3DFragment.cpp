// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "X3DFragment.h"
#include "engine/ObjectCollection.h"
#include "X3DFeatureInstance.h"
#include "X3DImageInstance.h"
#include <gz4d/X3D.h>
#include <gz4d/x3d/X3DNode.h>

REGISTER_OBJECT(X3DFragment, "x3df");

#include "MainWindow.h"
extern MainWindow *mainWindow;


//#include <gz4d/util/Xml.h>

bool X3DFragment::x3dInitialized = false;
//gz4d::X3D X3DFragment::x3d;
//template <class X3DFeatureInstance> gz4d::SFNode createX3DNode( Xml &x3d, X3D *root);
//template <class X3DImageInstance> gz4d::SFNode createX3DNode( Xml &x3d, X3D *root);

X3DFragment::X3DFragment():x3d(mainWindow->getGloContext())
{
    /*
    ObjectPoseState * op = new ObjectPoseState(this);
    pose = op;
    op->ignoreHeightExaggeration();
    */
    if (!x3dInitialized)
    {
        x3d.registerNodeType( "Feature",  gz4d::createX3DNode<X3DFeatureInstance>);
        x3d.registerNodeType( "Image",  gz4d::createX3DNode<X3DImageInstance>);
        x3dInitialized = true;
    }
    else
        x3d.Reset(false);
}

int X3DFragment::Load()
{
    if (x3d.LoadFragment(GetFilename()))
        return 0;
    else
        return 1;
}

void X3DFragment::publish(int parenti)
{
    gz4d::MFNode children;
    X3DPrototypeInstance *proto;
    Object *ob;

    Object::publish(parenti);

    //this is a temporary object.  All "children" will become children of the parent
    //add x3d child-objects as children.
    children = x3d.getRootNodes();
    for (gz4d::MFNode::iterator n = children.begin(); n != children.end(); n++)
    {
        DYNAMIC_CAST(proto, X3DPrototypeInstance *, &(**n));
        if (!proto)
            continue;
        ob = proto->getObject();
        if (ob)
        {
            ob->SetFilename(GetFilename());
            ob->requireFullReload();
            parent->AddObject(proto->getObject());
        }
    }
    //remove ourself.
    Unload();
}
