// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DSensorNode.h>

gz4d::X3DSensorNode::X3DSensorNode()
    :enabled(true)
{
}

void gz4d::X3DSensorNode::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d,"enabled",enabled);
    X3DChildNode::Load(x3d, root);
}
