// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/socket/IPSocket.h>
#include <iostream>

int main(int argc, char * argv[])
{
    std::vector<std::string> a = gz4d::IPSocket::Interfaces();
    for (std::vector<std::string>::iterator i = a.begin(); i!= a.end(); ++i)
        std::cout << *i << std::endl;
}
