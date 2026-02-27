#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DIndexedLineSet.h>

//#include <gz4d/util/Xml.h>
#include <gz4d/x3d/X3DCoordinate.h>
#include <gz4d/x3d/X3DColor.h>
#include <gz4d/X3D.h>

gz4d::X3DIndexedLineSet::X3DIndexedLineSet()
{
/*    color = NULL;
    coord = NULL;
    fogCoord= NULL;*/
    colorPerVertex = true;

    typeName = "IndexedLineSet";
}

void gz4d::X3DIndexedLineSet::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d, "colorPerVertex", colorPerVertex);
    //x3d.getBool( colorPerVertex, "creaseAngle");

    LoadArray( x3d, "colorIndex", colorIndex);
    LoadArray( x3d, "coordIndex", coordIndex);

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

void gz4d::X3DIndexedLineSet::Draw()
{
    if (preDraw())
        return;

    if (coordIndex.size() > 0)
    {
        SFInt32 lineIndex = 0;
        glBegin(GL_LINE_STRIP);
        for (SFInt32 cindex = 0; cindex < (SFInt32)(coordIndex.size()); cindex++)
        {
            if (coordIndex[cindex] == -1)
            {
                glEnd();
                lineIndex++;
                glBegin(GL_LINE_STRIP);
            }
            else if (coord)
            {
                if (color)
                {
                    if (colorPerVertex)
                    {
                        if ((SFInt32)(colorIndex.size()) > cindex)
                            color->Draw( colorIndex[cindex]);
                        else
                            color->Draw( coordIndex[cindex]);
                    }
                    else
                    {
                        if ((SFInt32)(colorIndex.size()) > lineIndex)
                            color->Draw( colorIndex[lineIndex]);
                        else
                            color->Draw( lineIndex);
                    }
                }

                coord->Draw(coordIndex[cindex]);
            }
        }
        glEnd();
    }
}
