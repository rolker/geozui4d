// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_NMEA_H
#define GZ4D_DATA_NMEA_H

#include <gz4d/data.h>
#include <gz4d/data/nmea/Sentence.h>
#include <gz4d/data/nmea/GGA.h>
#include <gz4d/data/nmea/HDG.h>
#include <gz4d/data/nmea/HDT.h>
#include <gz4d/data/nmea/PASHR.h>
#include <gz4d/data/nmea/ZDA.h>
#include <gz4d/data/nmea/RMC.h>
#include <gz4d/data/nmea/PSXN.h>
#include <gz4d/data/nmea/VDM.h>

namespace gz4d
{
    namespace nmea
    {
        class GZ4DEXPORT Exception: public gz4d::Exception
        {
            public:
                Exception(std::string msg):gz4d::Exception(msg){}
        };
    }
}

#endif
