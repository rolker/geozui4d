#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DCoordinate.h>
#include <gz4d/glo/Vertex.h>

gz4d::X3DCoordinate::X3DCoordinate()
{
    typeName = "Coordinate";
}

void gz4d::X3DCoordinate::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadArray( x3d, "point", point);
    X3DCoordinateNode::Load( x3d, root);
}

void gz4d::X3DCoordinate::Draw()
{
    if (preDraw())
        return;

    glVertexPointer(3,GL_FLOAT,0,&point.front().front());
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS,0,point.size());

}

void gz4d::X3DCoordinate::Draw(SFInt32 index)
{
    if (useNode)
        useNode->Draw(index);
    else
    {
        glo::Vertex(point[index]);
    }
}



