// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DGROUP_H
#define GZ4D_X3DGROUP_H

#include <gz4d/x3d/X3DGroupingNode.h>


namespace gz4d
{
    class GZ4DEXPORT X3DGroup: public X3DGroupingNode
    {
        public:
            X3DGroup();
            void Draw();
            SFNode newNode(std::string nodeType, boost::property_tree::ptree x3d, X3D* root);
    };
};

#endif
