// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DGEOORIGIN_H
#define GZ4D_X3DGEOORIGIN_H

#include "X3DNode.h"

namespace gz4d
{
    class X3DGeoOrigin: public X3DNode
    {
        public:
            X3DGeoOrigin();

            void Load(const boost::property_tree::ptree &x3d, X3D* root);
        protected:
            MFString geoSystem;
            SFBool rotateYUp;
    };
};

#endif
