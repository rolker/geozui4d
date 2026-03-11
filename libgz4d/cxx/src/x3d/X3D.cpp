// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/X3D.h>
#include <gz4d/glo/Error.h>
#include <gz4d/glo/PushAttrib.h>

gz4d::X3D::X3D(std::shared_ptr<glo::Context> c):context(c),showNormals(false)
{
}

gz4d::X3D::~X3D()
{
}

void gz4d::X3D::Reset(bool resetTypesToo)
{
    showNormals = false;
    displayList.Rebuild();
    filename = "";
    root.Clear();
    nodeList.clear();
    prototypeList.clear();
    if (resetTypesToo)
        nodeFactory.clear();
}

void gz4d::X3D::Draw()
{
    if(!displayList.Call())
    {
        {
            glo::PushAttrib attribs;
            glPushMatrix();
            glEnable(GL_LIGHTING);
            glEnable(GL_COLOR_MATERIAL);
            glShadeModel (GL_SMOOTH);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_NORMALIZE);
            root.Draw();
            glPopMatrix();
        }
        displayList.End();
    }
   glo::Error();
}


bool gz4d::X3D::Load(std::string const &fn)
{
    filename = fn;
    boost::property_tree::ptree doc;
    read_xml(fn,doc);
    
    boost::property_tree::ptree::const_assoc_iterator x3d = doc.find("X3D");

    if (x3d == doc.not_found())
        return false;
    for(boost::property_tree::ptree::const_iterator s = x3d->second.begin(); s != x3d->second.end(); ++s)
    {
        if(s->first == "Scene")
            root.Load(s->second, this);
    }
    displayList.Rebuild();
    return true;
}

bool gz4d::X3D::LoadFragment(std::string const &fn)
{
    filename = fn;
    boost::property_tree::ptree doc;
    read_xml(fn,doc);
    
    boost::property_tree::ptree::const_iterator x3d = doc.begin();

    root.LoadFragment(x3d, doc.end(), this);
    displayList.Rebuild();
    return true;
}

gz4d::SFNode gz4d::X3D::getNode(SFString nodeName)
{
    return nodeList[nodeName];
}

void gz4d::X3D::registerNode( SFString nodeName, SFNode node)
{
    nodeList[nodeName] = node;
}

void gz4d::X3D::registerProto( SFString name, ProtoRecord record )
{
    prototypeList[name] = record;
}

void gz4d::X3D::registerNodeType( SFString name, SFNodeCreator createMethod )
{
    nodeFactory[name] = createMethod;
}

gz4d::X3D::Ptr gz4d::X3D::New(std::shared_ptr<glo::Context> c)
{
    X3D::Ptr ret(new X3D(c));
    return ret;
}

