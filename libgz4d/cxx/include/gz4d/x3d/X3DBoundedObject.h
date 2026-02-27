#ifndef GZ4D_X3DBOUNDEDOBJECT_H
#define GZ4D_X3DBOUNDEDOBJECT_H

#include "X3DTypes.h"
#include "X3DAttributeContainer.h"

namespace gz4d
{

    class GZ4DEXPORT X3DBoundedObject : public X3DAttributeContainer
    {
    public:
        X3DBoundedObject();
        virtual void Load(const boost::property_tree::ptree &x3d, X3D* root);
    protected:
        SFVec3f bboxCenter;
        SFVec3f bboxSize;
    };
};

#endif

