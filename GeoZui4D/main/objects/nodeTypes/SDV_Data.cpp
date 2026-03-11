// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include <gz4d/gz4d.h>
#include "objects/nodeTypes/SDV_Data.h"
#include <gz4d/util/Utilities.h>

SDV_Data::SDV_Data()
{
    sounding[0] = sounding[1] = sounding[2] = 0.0;
    alternateData = gz4d::Nan<float>();
}
