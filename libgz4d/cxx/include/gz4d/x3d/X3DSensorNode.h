#ifndef GZ4D_X3DSENSORNODE_H
#define GZ4D_X3DSENSORNODE_H

#include "X3DChildNode.h"

namespace gz4d
{
    class GZ4DEXPORT X3DSensorNode: public X3DChildNode
    {
        public:
            X3DSensorNode();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);

        protected:
            SFBool enabled;
    };
}

#endif
