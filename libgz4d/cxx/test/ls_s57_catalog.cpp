// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/s57/Catalog.h>

int main(int argc, char *argv[])
{
    gz4d::s57::Catalog c(argv[1]);
    //c.printTileIndex();
    
    return 0;
}