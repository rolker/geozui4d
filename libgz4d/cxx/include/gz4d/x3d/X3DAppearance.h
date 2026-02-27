#ifndef GZ4D_X3DAPPEARANCE_H
#define GZ4D_X3DAPPEARANCE_H

#include "X3DAppearanceNode.h"
//#include <boost/property_tree/xml_parser.hpp>

namespace gz4d
{

class X3DAppearance : public X3DAppearanceNode
{
public:
    X3DAppearance();
    void Load(const boost::property_tree::ptree &x3d, X3D* root);
    virtual void Draw();

protected:
    SFNode fillProperties;
    SFNode lineProperties;
    SFNode material;
    SFNode texture;
    SFNode textureTransform;
};

};

#endif
