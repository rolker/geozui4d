// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DGroupingNode.h>
#include <gz4d/glo/Error.h>

void gz4d::X3DGroupingNode::addChildren(MFNode nodes)
{
    for (MFNode::iterator n = nodes.begin(); n != nodes.end(); n++)
        children.push_back(*n);
}

void gz4d::X3DGroupingNode::Clear()
{
    children.clear();
}

void gz4d::X3DGroupingNode::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    for(boost::property_tree::ptree::const_iterator n = x3d.begin(); n != x3d.end(); ++n)
    {
        SFNode newnode = newNode(n->first, n->second, root);
        if (newnode)
        {
            children.push_back(newnode);
        }
    }
    X3DBoundedObject::Load(x3d, root);
}

void gz4d::X3DGroupingNode::LoadFragment(boost::property_tree::basic_ptree< std::string, std::string >::const_iterator x3d, boost::property_tree::basic_ptree< std::string, std::string >::const_iterator end, gz4d::X3D* root)
{
    while(x3d != end)
    {
        SFNode newnode = newNode(x3d->first, x3d->second, root);
        if (newnode)
            children.push_back(newnode);
        ++x3d;
    }
}

void gz4d::X3DGroupingNode::Draw()
{

    if (preDraw())
        return;

    MFNode::iterator n = children.begin();
    while (n != children.end())
    {
        (*n)->Draw();
        glo::Error();
        n++;
    }
}
