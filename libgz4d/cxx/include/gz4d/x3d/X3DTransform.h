// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DTRANSFORM_H
#define GZ4D_X3DTRANSFORM_H

#include "X3DGroupingNode.h"
#include <gz4d/Translation.h>
#include <gz4d/Rotation.h>
#include <gz4d/Scale.h>

// http://www.web3d.org/x3d/specifications/ISO-IEC-19775-X3DAbstractSpecification/
// 
// The translation, rotation, scale, scaleOrientation and center fields define a geometric
// 3D transformation consisting of (in order):
// 
//    1. a (possibly) non-uniform scale about an arbitrary point;
//    2. a rotation about an arbitrary point and axis;
//    3. a translation.
// 
// The center field specifies a translation offset from the origin of the local coordinate
// system (0,0,0). The rotation field specifies a rotation of the coordinate system. The scale
// field specifies a non-uniform scale of the coordinate system. scale values may have any
// value: positive, negative (indicating a reflection), or zero. A value of zero indicates
// that any child geometry shall not be displayed. The scaleOrientation specifies a rotation
// of the coordinate system before the scale (to specify scales in arbitrary orientations).
// The scaleOrientation applies only to the scale operation. The translation field specifies
// a translation to the coordinate system.

namespace gz4d
{
    class X3DTransform: public X3DGroupingNode
    {
        Translation<float> center; ///< Offset relative to local coordinate system.
        Rotation<float> rotation; ///< Rotation of the coordinate system.
        Scale<float> scale;
        Rotation<float> scaleOrientation; ///< Rotation that applies only to the scale operation.
        Translation<float> translation; ///< Offset of the local coordinate system relative to the parent coordinate system.
    public:
        X3DTransform();

        void Load(const boost::property_tree::ptree &x3d, X3D* root);
        void Draw();
    };
};

#endif
