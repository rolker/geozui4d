// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DCOORDINATE_H
#define GZ4D_X3DCOORDINATE_H

#include "X3DCoordinateNode.h"

namespace gz4d
{

    class X3DCoordinate : public X3DCoordinateNode
    {
        public:
            X3DCoordinate();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();
            void Draw(SFInt32 index);
            SFVec3f getValue(SFInt32 i){return point[i];};
            std::size_t size(){return point.size();}
        protected:
            MFVec3f point;
    };
};

#endif
