// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DComposedGeometryNode.h>

#include <gz4d/x3d/X3DCoordinate.h>
#include <gz4d/x3d/X3DColor.h>
#include <gz4d/x3d/X3DNormal.h>
#include <gz4d/x3d/X3DTextureCoordinate.h>
#include <gz4d/glo/Normal.h>

gz4d::X3DComposedGeometryNode::X3DComposedGeometryNode()
{
    ccw = true;
    colorPerVertex = true;
    normalPerVertex = true;
    solid = true;
}

void gz4d::X3DComposedGeometryNode::Load(const boost::property_tree::ptree &x3d, X3D* root)
{

    LoadItem(x3d,"ccw",ccw);
    LoadItem(x3d,"colorPerVertex",colorPerVertex);
    LoadItem(x3d,"normalPerVertex",normalPerVertex);
    LoadItem(x3d,"solid",solid);

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

        if (n->first == "Normal")
        {
            normal = SFNode(new X3DNormal, n->second, root);
        }

        if (n->first == "TextureCoordinate")
        {
            texCoord = SFNode(new X3DTextureCoordinate, n->second, root);
        }

    }
    X3DGeometryNode::Load( x3d, root);
}

void gz4d::X3DComposedGeometryNode::Draw()
{
    if (preDraw())
        return;
    if (coord)
    {
        coord->Draw();
    }
    glPopAttrib();
}

void gz4d::X3DComposedGeometryNode::Draw(SFInt32 index)
{
    if (coord)
    {
        coord->Draw(index);
    }
}

void gz4d::X3DComposedGeometryNode::DrawNormal(SFInt32 index, SFInt32 ni)
{
    if (coord && normal)
    {
        ((X3DNormal*) normal.get())->Draw(ni, ((X3DCoordinate*)coord.get())->getValue(index));
    }
}

void gz4d::X3DComposedGeometryNode::ApplyNormal(SFInt32 index)
{
    if (normal)
        normal->Draw(index);
    else
    {
        if(generated_normals.empty())
            GenerateNormals();
        glo::Normal(generated_normals[index]);
    }
}

void gz4d::X3DComposedGeometryNode::ApplyColor(SFInt32 index)
{
    if (color)
        color->Draw(index);
}

void gz4d::X3DComposedGeometryNode::ApplyTexCoord(SFInt32 index)
{
    if (texCoord)
        texCoord->Draw(index);
}
