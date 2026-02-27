#ifndef GZ4D_X3DCONE_H
#define GZ4D_X3DCONE_H

#include "X3DGeometryNode.h"

namespace gz4d
{
    class X3DCone : public X3DGeometryNode
    {
        public:
            X3DCone();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();
        protected:
            SFBool bottom;
            SFFloat height;
            SFFloat bottomRadius;
            SFBool side;
            SFBool solid;
    };
};

#endif
