// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/data/Binary.h>
#include <cstdio>
#include <iostream>

int main(int argc, char* argv[])
{
    FILE* outfile;
    if (argc >= 2)
        outfile = fopen(argv[1],"wb");
    else
        outfile = fopen("test.gz4d","wb");

    gz4d::binary::Navigation::Position::Geographic nav;
    nav.latitude = 43.2314;
    nav.longitude = -73.24653;

    fwrite(&nav, nav.size, 1, outfile);

    gz4d::binary::Wrapper<gz4d::binary::Sonar::Channel::BottomDetection> bd;
    std::cerr << bd->size << std::endl;
    bd->samples.size = 2;
    bd.Resize();
    std::cerr << bd->size << std::endl;
    bd->samples.values[0] = 1.0;
    bd->samples.values[1] = -20.0;
    bd->samples.size = 4;
    bd.Resize();
    std::cerr << bd->size << std::endl;
    bd->samples.values[2] = 99;
    bd->samples.values[3] = .005;
    fwrite (&(*bd), bd->size, 1, outfile);
}
