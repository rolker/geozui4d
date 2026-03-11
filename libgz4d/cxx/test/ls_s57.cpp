// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/s57/S57.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <gz4d/vector/ios.h>

int main(int argc, char * argv[])
{
    gz4d::s57::S57 enc(argv[1]);
    std::cout << enc.getBBox().getMin() << "\t" << enc.getBBox().getMax() << std::endl;
/*    if(argc >= 6)
    {
        gz4d::Box<float,3> bbox;
        gz4d::Vector<float,3> p;
        p[1] = boost::lexical_cast<float>(argv[2]);
        p[0] = boost::lexical_cast<float>(argv[3]);
        bbox.expand(p);

        p[1] = boost::lexical_cast<float>(argv[4]);
        p[0] = boost::lexical_cast<float>(argv[5]);
        bbox.expand(p);

        gz4d::s57::Cell::List cl = enc.intersects(bbox);
        for (gz4d::s57::Cell::List::iterator c = cl.begin(); c != cl.end(); ++c)
            std::cout << (*c)->info() << std::endl;
    }*/
}

