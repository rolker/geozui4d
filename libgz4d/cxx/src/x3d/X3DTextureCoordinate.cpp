#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DTextureCoordinate.h>
#include <iostream>

gz4d::X3DTextureCoordinate::X3DTextureCoordinate()
{
    typeName = "TextureCoordinate";
}

void gz4d::X3DTextureCoordinate::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadArray( x3d, "point", point);
    X3DTextureCoordinateNode::Load( x3d, root);
}

void gz4d::X3DTextureCoordinate::Draw(SFInt32 index)
{
    if (index < (SFInt32)(point.size()))
    {
        glTexCoord2fv(&point[index][0]);
    }
}
