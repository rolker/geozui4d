#ifndef GZ4D_X3DATTRIBUTECONTAINER_H
#define GZ4D_X3DATTRIBUTECONTAINER_H

#include <string>
#include "X3DTypes.h"
#include <boost/property_tree/xml_parser.hpp>

namespace gz4d
{
    class X3D;

    class GZ4DEXPORT X3DAttributeContainer
    {
    public:
        virtual ~X3DAttributeContainer();
        virtual void Load(const boost::property_tree::ptree &x3d, X3D* root) = 0;
    };

};

#endif
