// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/image/io.h>
#include <iostream>

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
        std::cerr << "usage: image_info image\n";
        exit(1);
    }
    gz4d::image::Info i = gz4d::image::info(argv[1]);
    std::cout << argv[1] << " " << i.width << " x " << i.height << ", " << i.format << std::endl;
}
