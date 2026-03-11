// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gtest/gtest.h>

#include "../include/gz4d/Box.h"
#include "../include/gz4d/geo/Position.h"

TEST(box,GeoPositions)
{
    gz4d::Box<gz4d::geo::Position<double> > box;
    
    EXPECT_TRUE(box.empty());
    
    gz4d::geo::Position<double> p1(80.0,-165);
    box.expand(p1);
    
    EXPECT_FALSE(box.empty());
    
    EXPECT_EQ(p1,box.getMin());
    EXPECT_EQ(p1,box.getMax());

    gz4d::geo::Position<double> p2(85.0,-170);
    box.expand(p2);

    EXPECT_EQ(gz4d::geo::Position<double>(5.0,5.0),box.getSizes());

    gz4d::geo::Position<double> p3(75.0,178);
    box.expand(p3);
    
    EXPECT_EQ(gz4d::geo::Position<double>(75.0,178.0),box.getMin());
    EXPECT_EQ(gz4d::geo::Position<double>(85.0,-165.0),box.getMax());
    
}
