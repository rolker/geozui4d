#ifndef GZ4D_X3DSHAPENODE_H
#define GZ4D_X3DSHAPENODE_H

#include "X3DChildNode.h"
#include "X3DBoundedObject.h"

namespace gz4d
{
    class X3DShapeNode: public X3DChildNode, public X3DBoundedObject
    {
        public:
            X3DShapeNode();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            virtual void Draw();
        protected:
            SFNode appearance;
            SFNode geometry;
    };
};

#endif
