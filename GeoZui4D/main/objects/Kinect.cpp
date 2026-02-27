#include "Kinect.h"

#include <gz4d/Matrix.h>
#include <gz4d/glo/Color.h>
#include <gz4d/glo/Vertex.h>
#include "engine/InputMediator.h"
#include "engine/Scene3D.h"
#include <gz4d/glo/PushAttrib.h>
#include "engine/ObjectPoseState.h"

REGISTER_OBJECT(Kinect, "");

Kinect::Kinect()
{
    ObjectPoseState * op = new ObjectPoseState(this);
    pose = op;
    op->ignoreHeightExaggeration();
}

void Kinect::DrawLowRes(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT|GL_LIGHTING_BIT);
    glPointSize(2);
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glRotatef(90.0,1.0,0.0,0.0);

    if(!pointCloud.empty())
    {
        glInterleavedArrays(GL_C3F_V3F,0,&pointCloud.front());
        glDrawArrays(GL_POINTS,0,pointCloud.size());
    }

    glPopMatrix();
}

void Kinect::Idle(double time)
{
    if(isVisible())
    {
        pointCloud = kinect.getPointCloud();
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
    Object::Idle(time);
}

