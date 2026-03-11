// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <gz4d/nav/Track.h>

int main(int argc, char *argv[])
{
    gz4d::nav::Track t;
    
    for(int i = 1; i < argc; ++i)
    {
        std::cout << "loading " << argv[i] << std::endl;
        t.loadFile(argv[i]);
    }
    std::cout << "done loading.\n" << t.getInfo() << std::endl;
    exit(0);
}