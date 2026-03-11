// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DGROUPINGNODE_H
#define GZ4D_X3DGROUPINGNODE_H

#include "X3DChildNode.h"
#include "X3DBoundedObject.h"

namespace gz4d
{
    class GZ4DEXPORT X3DGroupingNode: public X3DChildNode, X3DBoundedObject
    {
    public:
        void addChildren(MFNode nodes);
        void removeChildren(MFNode nodes);
    //    bool setValue(std::string attribute, std::string value);
        virtual void Load(const boost::property_tree::ptree &x3d, X3D* root);
        virtual void LoadFragment(boost::property_tree::basic_ptree< std::string, std::string >::const_iterator x3d, boost::property_tree::basic_ptree< std::string, std::string >::const_iterator end, gz4d::X3D* root);
        virtual void Draw();
        MFNode const &getChildren() {return children;}

        virtual void Clear();

    protected:
        MFNode children;
    };
}

#endif
