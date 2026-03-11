// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3D_NODEDATA_H
#define GZ4D_X3D_NODEDATA_H

#include <gz4d/gz4d.h>
#include <gz4d/x3d/sai/Node.h>
#include <map>

namespace gz4d
{

    namespace x3d
    {
        class FieldValue;

        class GZ4DEXPORT NodeData
        {
            protected:
                std::map<std::string,std::shared_ptr<FieldValue> > fields;
            public:
                virtual ~NodeData();
                virtual std::string getTypeName() const =0;
                virtual void addNode(sai::Node n) =0;
                virtual void set_field(std::string const &f, std::string const &v) =0;
        };
    }
}

#endif
