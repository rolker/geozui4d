// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DLineProperties.h>
//#include <gz4d/util/Xml.h>

unsigned short gz4d::X3DLineProperties::stipplings[User_specified_dash + 1] = {
    0xFFFF, //Nothing--use Solid
    0xFFFF, //Solid
    0x3F3F, //Dashed
    0x1111, //Dotted
    0x11FF, //Dash_dotted
    0x24FF, //Dash_dot_dot
    0xFFFF, //Single_arrow--Not implemented, using Solid
    0xFFFF, //Single_dot--Not implemented, using Solid
    0xFFFF, //Double_arrow--Not implemented, using Solid
    0xFFFF, //Stitch_line--Not implemented, using Solid
    0xFFFF, //Chain_line--Not implemented, using Solid
    0xFFFF, //Center_line--Not implemented, using Solid
    0xFFFF, //Hidden_line--Not implemented, using Solid
    0xFFFF, //Phantom_line--Not implemented, using Solid
    0xFFFF, //Break_line_1--Not implemented, using Solid
    0xFFFF, //Break_line_2--Not implemented, using Solid
    0xFFFF  //User_specified_dash--Not implemented, using Solid
};

gz4d::X3DLineProperties::X3DLineProperties()
{
    applied = true;
    linetype = Solid;
    linewidthScaleFactor = 0.0;
    typeName = "LineProperties";

    stippling = false;
}

void gz4d::X3DLineProperties::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d,"applied",applied);
    LoadItem(x3d,"linetype",linetype);
    LoadItem(x3d,"linewidthScaleFactor",linewidthScaleFactor);
/*    x3d.getBool( applied, "applied");
    x3d.getInt( linetype, "linetype");
    x3d.getFloat( linewidthScaleFactor, "linewidthScaleFactor");*/
    if (linetype < Solid || linetype > User_specified_dash)
        linetype = 1;
    if (linewidthScaleFactor < 0.0)
        linewidthScaleFactor = 0.0;
    X3DAppearanceChildNode::Load( x3d, root);
}

void gz4d::X3DLineProperties::Draw()
{
    if (preDraw())
        return;

    if (applied)
    {
        glLineWidth(linewidthScaleFactor);
        if (linetype != Solid)
        {
            glLineStipple((int)linewidthScaleFactor, stipplings[linetype]);
            glEnable(GL_LINE_STIPPLE);
            stippling = true;
        }
    }
}
