// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DBoundedObject.h>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <gz4d/vector/ios.h>

gz4d::X3DBoundedObject::X3DBoundedObject()
    :bboxCenter(0.0),bboxSize(-1.0)
{
}

void gz4d::X3DBoundedObject::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    if(x3d.count("<xmlattr>"))
    {
        boost::optional<std::string> a = x3d.get_optional<std::string>("<xmlattr>.bboxCenter");
        if (a)
            bboxCenter = boost::lexical_cast<SFVec3f>(a.get());
        a = x3d.get_optional<std::string>("<xmlattr>.bboxSize");
        if (a)
            bboxSize = boost::lexical_cast<SFVec3f>(a.get());
    }
}
