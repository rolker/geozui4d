// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/X3D.h>
#include <gz4d/x3d/X3DPrototypeRecord.h>
#include <iostream>

gz4d::X3DPrototypeRecord::X3DPrototypeRecord()
{
    createMethod = NULL;
    typeName = "X3DPrototypeRecord";
}

void gz4d::X3DPrototypeRecord::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    X3DNode::LoadArray( x3d, "url", url );
    if (url[0].compare(0, 7, "gz4d://") != 0) //not an internal thing!
    {
        std::cerr << "ExternProtoDeclare currently only recognizes internal types (beginning with gz4d://)" << std::endl;
        return;
    }
    createMethod = root->getCreateMethod(url[0].substr(7));
}

gz4d::SFNode gz4d::X3DPrototypeRecord::createInstance(boost::property_tree::ptree x3d, X3D* root)
{
    if (createMethod)
        return createMethod(x3d, root);
    else
        return NULL;
}

gz4d::ProtoRecord::ProtoRecord(boost::property_tree::ptree x3d, X3D* root)
: std::shared_ptr<X3DPrototypeRecord>(new X3DPrototypeRecord)
{
    (*this)->Load(x3d, root);
}
