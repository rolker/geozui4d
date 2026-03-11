// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gz4d/socket/BTSocketManager.h>
#include<iostream>
#include<string>

int main(int argc, char * argv[])
{
    gz4d::BTSocketManager bm;
    std::vector<gz4d::BTDevice> dl = bm.Scan();
    std::cout << dl.size() << " devices\n";
    for (std::vector<gz4d::BTDevice>::iterator i = dl.begin(); i != dl.end(); ++i)
    {
        std::cout << i->name << ", " << i->address << std::endl;
    }
    return 0;
}
