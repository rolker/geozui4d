// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef OGR_H
#define OGR_H

#include "engine/ObjectCollection.h"

class OGR : public ObjectCollection
{
    public:
        OGR();

        DERIVE_OBJECT(OGR);

        Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
};

#endif
