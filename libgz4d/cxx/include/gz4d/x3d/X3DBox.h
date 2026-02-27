#ifndef GZ4D_X3DBOX_H
#define GZ4D_X3DBOX_H

#include <gz4d/x3d/X3DGeometryNode.h>

namespace gz4d
{
    class X3DBox : public X3DGeometryNode
    {
        public:
            X3DBox();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();
        protected:
            SFVec3f size;
            SFBool solid;
            SFVec3f pos;
    };
};
#endif
