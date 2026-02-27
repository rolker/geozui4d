#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DPointSet.h>

#include <gz4d/x3d/X3DCoordinate.h>
#include <gz4d/x3d/X3DColor.h>
#include <gz4d/X3D.h>
#include <gz4d/glo/Color.h>

gz4d::X3DPointSet::X3DPointSet()
{
    typeName = "PointSet";
}

void gz4d::X3DPointSet::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    for(boost::property_tree::ptree::const_iterator n = x3d.begin(); n != x3d.end(); ++n)
    {
        if (n->first == "Coordinate")
        {
            coord = SFNode(new X3DCoordinate, n->second, root);
        }

        if (n->first == "Color")
        {
            color = SFNode(new X3DColor, n->second, root);
        }

    }
    X3DGeometryNode::Load( x3d, root);
}

void gz4d::X3DPointSet::Draw()
{
    if (preDraw())
        return;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_3D);
    glDisable(GL_TEXTURE_CUBE_MAP);
    if (coord)
    {
        if(color)
            color->Draw();
        coord->Draw();

    }
}
