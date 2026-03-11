// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DIMAGETEXTURE_H
#define GZ4D_X3DIMAGETEXTURE_H

#include "X3DTexture2DNode.h"

#include <gz4d/Image.h>
#include <gz4d/glo/Texture.h>

namespace gz4d
{
    class X3DImageTexture : public X3DTexture2DNode
    {
        public:
            X3DImageTexture(glo::Context::Ptr c);
            void Load(const boost::property_tree::ptree& x3d, X3D* root);
            void Draw();
        protected:
            MFString url;
            gz4d::ImageL8 image_l;
            gz4d::ImageLA8 image_la;
            gz4d::ImageRGB8 image_rgb;
            gz4d::ImageRGBA8 image_rgba;
            gz4d::glo::Texture texture;
    };
};

#endif
