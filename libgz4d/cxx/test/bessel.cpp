// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gz4d/util/Bessel.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
    double x;
    double inc;
    int count = 1;

    switch (argc)
    {
        case 4:
            count = atoi(argv[2]);
            inc = atof(argv[3]);
        case 2:
            x = atof(argv[1]);
            break;
        default:
            std::cerr << "Usage: bessel x [count increment]\n";
            exit(1);
    }

    gz4d::Bessel<0> J0;
    gz4d::Bessel<1> J1;
    gz4d::Bessel<2> J2;
    for(int i = 0; i<count; ++i)
    {
        std::cout << "J0" << "(" << x+inc*i << ")=" << J0(x+inc*i) << "\t";
        std::cout << "J1" << "(" << x+inc*i << ")=" << J1(x+inc*i) << "\t";
        std::cout << "J2" << "(" << x+inc*i << ")=" << J2(x+inc*i) << "\t";
    }
}
