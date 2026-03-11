// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DPROTOTYPERECORD_H
#define GZ4D_X3DPROTOTYPERECORD_H

#include <gz4d/x3d/X3DTypes.h>
#include <gz4d/x3d/X3DNode.h>

namespace gz4d
{
    class X3D;
    class GZ4DEXPORT X3DPrototypeRecord
    {
        public:
            X3DPrototypeRecord();
            ~X3DPrototypeRecord() {};

            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            SFNode createInstance(boost::property_tree::ptree x3d, X3D* root);

    protected:
            SFNodeCreator createMethod; //pointer to a creation method
            SFString typeName;
            MFString url;
    };
    class ProtoRecord: public std::shared_ptr<X3DPrototypeRecord>
    {
        public:
            ProtoRecord(){}
            ProtoRecord(boost::property_tree::ptree x3d, X3D* root);
            ProtoRecord(X3DPrototypeRecord *n) : std::shared_ptr<X3DPrototypeRecord>(n){}
    };
};

#endif

