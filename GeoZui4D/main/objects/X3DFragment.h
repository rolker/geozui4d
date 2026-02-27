#ifndef X3DFRAGMENT_H
#define X3DFRAGMENT_H

#include "engine/Object.h"
#include <gz4d/X3D.h>

class X3DFragment: public Object
{
public:
    X3DFragment();
    ~X3DFragment() {};

    DERIVE_OBJECT(X3DFragment);
    void DrawLowRes(float degradation, Window3D *win) {};
    int Load();
    virtual void publish(int parent);

private:
    //static gz4d::X3D x3d;
    gz4d::X3D x3d;
    static bool x3dInitialized;
};

#endif
