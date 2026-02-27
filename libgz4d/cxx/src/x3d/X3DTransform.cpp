#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DTransform.h>
#include <gz4d/glo/MultMatrix.h>
#include <cmath>


gz4d::X3DTransform::X3DTransform()
{
    typeName = "Transform";
}

void gz4d::X3DTransform::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    SFRotation r(0.0,0.0,1.0,0.0);
    SFRotation so(0.0,0.0,1.0,0.0);

    LoadItem(x3d,"center",center);
    LoadItem(x3d,"rotation",r);
    LoadItem(x3d,"scale",scale);
    LoadItem(x3d,"scaleOrientation",so);
    LoadItem(x3d,"translation",translation);

    rotation = Rotation<float>(Angle<float,Radian>(r[3]),Point<float>(r[0],r[1],r[2]));
    scaleOrientation = Rotation<float>(Angle<float,Radian>(so[3]),Point<float>(so[0],so[1],so[2]));

    X3DGroupingNode::Load(x3d, root);
}

void gz4d::X3DTransform::Draw()
{
    if (preDraw())
        return;

    glPushMatrix();

    glo::MultMatrix(translation.GetMatrix()*center.GetMatrix()*rotation.GetMatrix()*scaleOrientation.GetMatrix()*scale.GetMatrix()*scaleOrientation.GetInverseMatrix()*center.GetInverseMatrix());

    X3DGroupingNode::Draw();

    glPopMatrix();
}

