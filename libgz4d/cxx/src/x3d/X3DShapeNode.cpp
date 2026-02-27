#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DShapeNode.h>
#include <gz4d/glo/Error.h>

#include <gz4d/x3d/X3DAppearance.h>
#include <gz4d/x3d/X3DIndexedFaceSet.h>
#include <gz4d/x3d/X3DIndexedLineSet.h>
#include <gz4d/x3d/X3DBox.h>
#include <gz4d/x3d/X3DCylinder.h>
#include <gz4d/x3d/X3DCone.h>
#include <gz4d/x3d/X3DSphere.h>
#include <gz4d/x3d/X3DPointSet.h>

gz4d::X3DShapeNode::X3DShapeNode()
{
}

void gz4d::X3DShapeNode::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    for(boost::property_tree::ptree::const_iterator n = x3d.begin(); n != x3d.end(); ++n)
    {
        if (n->first == "Appearance")
        {
            appearance = SFNode(new X3DAppearance(),n->second, root);
        }

        if (n->first == "IndexedFaceSet")
        {
            geometry = SFNode(new X3DIndexedFaceSet(),n->second, root);
        }

        if (n->first == "IndexedLineSet")
        {
            geometry = SFNode(new X3DIndexedLineSet(),n->second, root);
        }

        if (n->first == "Box")
        {
            geometry = SFNode(new X3DBox(),n->second, root);
        }

        if (n->first == "Cylinder")
        {
            geometry = SFNode(new X3DCylinder(),n->second, root);
        }

        if (n->first == "Cone")
        {
            geometry = SFNode(new X3DCone(),n->second, root);
        }

        if (n->first == "Sphere")
        {
            geometry = SFNode(new X3DSphere(),n->second, root);
        }

        if (n->first == "PointSet")
        {
            geometry = SFNode(new X3DPointSet(),n->second, root);
        }
    }

    X3DChildNode::Load( x3d, root);
    X3DBoundedObject::Load( x3d, root);
}

void gz4d::X3DShapeNode::Draw()
{
    if (preDraw())
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (appearance)
    {
        appearance->Draw();
        glo::Error();
    }
    else
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glColor3f(1.0,1.0,1.0);
    }

    if (geometry)
    {
        geometry->Draw();
        glo::Error();
    }
    glPopAttrib();

}
