#ifndef GZ4D_X3DTEXTURECOORDINATE
#define GZ4D_X3DTEXTURECOORDINATE

#include "X3DTextureCoordinateNode.h"

namespace gz4d
{
    class X3DTextureCoordinate : public X3DTextureCoordinateNode
    {
        public:
            X3DTextureCoordinate();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw(SFInt32 index);
        protected:
            MFVec2f point;
    };
};

#endif
