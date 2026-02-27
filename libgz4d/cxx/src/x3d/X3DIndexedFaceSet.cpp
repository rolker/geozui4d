#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DIndexedFaceSet.h>

#include <string>
#include <sstream>
#include <gz4d/X3D.h>
#include <gz4d/x3d/X3DCoordinate.h>
#include <gz4d/vector/operations.h>
#include <gz4d/glo/Normal.h>

gz4d::X3DIndexedFaceSet::X3DIndexedFaceSet()
{
    creaseAngle = 0.0;
    typeName = "IndexedFaceSet";
    r = NULL;
}

void gz4d::X3DIndexedFaceSet::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    r = root;

    LoadItem(x3d,"creaseAngle",creaseAngle);

    LoadArray( x3d, "colorIndex", colorIndex);
    LoadArray( x3d, "coordIndex", coordIndex);
    LoadArray( x3d, "normalIndex", normalIndex);
    LoadArray( x3d, "texCoordIndex", texCoordIndex);

    X3DComposedGeometryNode::Load( x3d, root);
}

void gz4d::X3DIndexedFaceSet::Draw()
{
    if (preDraw())
        return;

    bool texIndexFlag(false);

    if (texCoord)
    {
        if (texCoordIndex.size() > 0)
            texIndexFlag = true;
    }

    glPushAttrib(GL_LIGHTING_BIT|GL_POLYGON_BIT);
    
    if(color)
    {
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    }


    if(solid)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else
    {
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
        glDisable(GL_CULL_FACE);
    }

    if (ccw)
        glFrontFace(GL_CCW);
        //glFrontFace(GL_CW); // hack for inverted screen
    else
        //glFrontFace(GL_CCW);
        glFrontFace(GL_CW);

    if (coordIndex.size() > 0)
    {
        SFInt32 faceIndex = 0;
        glBegin(GL_POLYGON);
        for (SFInt32 cindex = 0; cindex < (SFInt32)(coordIndex.size()); cindex++)
        {
            if (coordIndex[cindex] == -1)
            {
                glEnd();
                faceIndex++;
                glBegin(GL_POLYGON);
            }
            else
            {
                if (normalPerVertex)
                {
                    if(!normal)
                    {
                        if(generated_normals.empty())
                            GenerateNormals();
                        Vector<float,3> n = generated_normals[faceIndex];
                        SFInt32 sf = 0;
                        for (SFInt32 i = 0; i < (SFInt32)(coordIndex.size()); i++)
                        {
                            if (coordIndex[i] == -1)
                                sf++;
                            else
                            {
                                if(i != cindex && coordIndex[i] == coordIndex[cindex])
                                    if(generated_normals[faceIndex].dot(generated_normals[sf]) > cos(creaseAngle))
                                        n += generated_normals[sf];
                            }
                        }
                        glo::Normal(normalize(n));
                    }
                    else
                        if ((SFInt32)(normalIndex.size()) > cindex)
                            ApplyNormal( normalIndex[cindex]);
                        else
                            ApplyNormal( coordIndex[cindex]);
                }
                else
                {
                    if ((SFInt32)(normalIndex.size()) > faceIndex)
                        ApplyNormal( normalIndex[faceIndex]);
                    else
                        ApplyNormal( faceIndex);
                }

                if (colorPerVertex)
                {
                    if ((SFInt32)(colorIndex.size()) > cindex)
                        ApplyColor( colorIndex[cindex]);
                    else
                        ApplyColor( coordIndex[cindex]);
                }
                else
                {
                    if ((SFInt32)(colorIndex.size()) > faceIndex)
                        ApplyColor( colorIndex[faceIndex]);
                    else
                        ApplyColor( faceIndex);
                }

                if (texCoord)
                {
                    if (texIndexFlag)
                        ApplyTexCoord( texCoordIndex[cindex]);
                    else
                        ApplyTexCoord( coordIndex[cindex]);
                }
                X3DComposedGeometryNode::Draw(coordIndex[cindex]);
            }
        }
        glEnd();
    }
    glPopAttrib();

    if (r && r->getShowNormals() && coordIndex.size() > 0)
    {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        for (SFInt32 cindex = 0; cindex < (SFInt32)(coordIndex.size()); cindex++)
        {
            if (coordIndex[cindex] == -1)
            {
                glEnd();
                glBegin(GL_LINES);
            }
            else
            {
                X3DComposedGeometryNode::Draw(coordIndex[cindex]);
                X3DComposedGeometryNode::DrawNormal(coordIndex[cindex],normalIndex[cindex]);
            }
        }
        glEnd();
    }

}

void gz4d::X3DIndexedFaceSet::GenerateNormals()
{
    for (SFInt32 cindex = 0; cindex < (SFInt32)(coordIndex.size()); cindex++)
    {
        SFVec3f v0 = std::dynamic_pointer_cast<X3DCoordinate>(coord)->getValue(coordIndex[cindex]);
        SFVec3f v1 = std::dynamic_pointer_cast<X3DCoordinate>(coord)->getValue(coordIndex[cindex + 1]);
        SFVec3f v2 = std::dynamic_pointer_cast<X3DCoordinate>(coord)->getValue(coordIndex[cindex + 2]);
        Vector<float,3> n = cross(v2-v1,v0-v1);
        if(norm(n) > 0)
        {
            generated_normals.push_back(normalize(n));
            while(cindex < (SFInt32)(coordIndex.size()) && coordIndex[cindex] != -1)
                ++cindex;
        }
    }
}
