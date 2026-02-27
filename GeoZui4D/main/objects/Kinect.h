#ifndef KINECT_H
#define KINECT_H

#include <gz4d/Kinect.h>
#include "engine/Object.h"
#include <gz4d/Matrix.h>

class Kinect : public Object
{
    gz4d::Kinect kinect;
    gz4d::Kinect::PointCloud pointCloud;
    public:
        Kinect();
        DERIVE_OBJECT(Kinect);
        void DrawLowRes(float degradation, Window3D *win);
        void Idle(double time);
};



#endif
