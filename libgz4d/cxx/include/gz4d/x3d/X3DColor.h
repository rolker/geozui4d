#ifndef GZ4D_X3DCOLOR_H
#define GZ4D_X3DCOLOR_H

#include "X3DColorNode.h"

namespace gz4d
{

    class X3DColor : public X3DColorNode
    {
        public:
            X3DColor();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();
            void Draw(SFInt32 index);
            SFColor getValue(SFInt32 i){return color[i];};
        protected:
            MFColor color;
    };
};

#endif
