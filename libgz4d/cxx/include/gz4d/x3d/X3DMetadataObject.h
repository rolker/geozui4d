// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DMETADATAOBJECT_H
#define GZ4D_X3DMETADATAOBJECT_H

#include "X3DTypes.h"

namespace gz4d
{

    class Xml;

    class X3DMetadataObject
    { 
        public:
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
        protected:
            SFString name;
            SFString reference;
    };
};

#endif
