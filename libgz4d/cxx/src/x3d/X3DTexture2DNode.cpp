#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DTexture2DNode.h>

//#include <gz4d/util/Xml.h>

gz4d::X3DTexture2DNode::X3DTexture2DNode()
{
    repeatS = true;
    repeatT = true;
}

void gz4d::X3DTexture2DNode::Load(const boost::property_tree::ptree& x3d, gz4d::X3D* root)
{
    LoadItem(x3d,"repeatS",repeatS);
    LoadItem(x3d,"repeatT",repeatT);
/*    x3d.getBool( repeatS,"repeatS");
    x3d.getBool( repeatT,"repeatT");*/
    X3DTextureNode::Load( x3d, root);
}
