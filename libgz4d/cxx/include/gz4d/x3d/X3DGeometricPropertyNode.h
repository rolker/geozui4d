#ifndef X3DGEOMETRICPROPERTYNODE_H
#define X3DGEOMETRICPROPERTYNODE_H

#include "X3DNode.h"

namespace gz4d
{
    class X3DGeometricPropertyNode : public X3DNode
    {
        public:
            virtual void Draw(SFInt32 index){};
    };
};

#endif
