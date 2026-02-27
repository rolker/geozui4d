#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DGroup.h>
#include <gz4d/X3D.h>
#include <gz4d/glo/Error.h>
#include <iostream>

gz4d::X3DGroup::X3DGroup()
{
    typeName = "Group";
}

void gz4d::X3DGroup::Draw()
{
    if (preDraw())
        return;
    glo::Error();
    glPushMatrix();
    X3DGroupingNode::Draw();
    glPopMatrix();
}

gz4d::SFNode gz4d::X3DGroup::newNode(std::string nodeType, boost::property_tree::ptree x3d, X3D* root)
{
    //---Non-node elements (that will return NULL)---
    if(nodeType == "ExternProtoDeclare")
    {
        boost::optional<std::string> a = x3d.get_optional<std::string>("<xmlattr>.name");
        //SFString protoName;
        if (a)
            root->registerProto( a.get(), ProtoRecord(x3d, root));
        else
        {
            std::cerr << "No name given for ExternProtoDeclare!" << std::endl;
        }
        return NULL;
    }
    else if (nodeType == "ProtoDeclare")
    {
        std::cerr << "X3D ProtoDeclare element not implemented at this time." << std::endl;
    }
    //---Nothing special, lets pass it on!---
    return X3DGroupingNode::newNode(nodeType, x3d, root);
}

