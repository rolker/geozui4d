// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/data/NMEA.h>
#include <iostream>

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        gz4d::nmea::Sentence nmea(argv[i]);
        std::cerr << nmea();
        for(int j = 0; j < nmea.size(); ++j)
            std::cerr << nmea[j] << std::endl;
    }
    return 0;
}
