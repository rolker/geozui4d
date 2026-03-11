// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/Image.h>
#include <gz4d/image/io.h>
#include <iostream>

int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        std::cerr << "usage: cp_image in out\n";
        exit(1);
    }
    gz4d::ImageRGBA8 i;
    //gz4d::ImageL8 i;
    i.Load(argv[1]);
    i.Save(argv[2]);
}
