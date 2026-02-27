#ifndef GZ4D_X3DPOINTSET
#define GZ4D_X3DPOINTSET

#include "X3DGeometryNode.h"

namespace gz4d
{
    class X3DPointSet : public X3DGeometryNode
    {
        public:
            X3DPointSet();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            virtual void Draw();
        protected:
            MFNode  attrib;
            SFNode  color;
            SFNode  coord;
            SFNode  fogCoord;
    };
};

#endif
