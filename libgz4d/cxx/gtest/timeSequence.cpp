// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gtest/gtest.h>

#include "../include/gz4d/TimeSequence.h"

TEST(TimeSequence,interpolate)
{
    gz4d::TimeSequence<double> ts;
    
    ts[gz4d::Time(1)]=1.0;
    ts[gz4d::Time(2)]=2.0;
    ts[gz4d::Time(3)]=3.0;
    ts[gz4d::Time(4)]=4.0;
    ts[gz4d::Time(5)]=5.0;
    
    // typical in between values within range
    EXPECT_EQ(3.75,ts.GetInterpolated(gz4d::Time(3.75)));
    EXPECT_EQ(1.5,ts.GetInterpolated(gz4d::Time(1.5)));
    
    // out of range
    EXPECT_EQ(1.0,ts.GetInterpolated(gz4d::Time(-1.5)));
    EXPECT_EQ(5.0,ts.GetInterpolated(gz4d::Time(9.5)));

    // values equal to values in sequence, including end values
    EXPECT_EQ(1.0,ts.GetInterpolated(gz4d::Time(1.0)));
    EXPECT_EQ(3.0,ts.GetInterpolated(gz4d::Time(3.0)));
    EXPECT_EQ(5.0,ts.GetInterpolated(gz4d::Time(5.0)));
}
