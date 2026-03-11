// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gtest/gtest.h>

#include "../include/gz4d/Angle.h"

TEST(angle,DegreePositivePeriod)
{
    gz4d::Angle<double,gz4d::Degree,gz4d::PositivePeriod> a1(0.0);
    gz4d::Angle<double,gz4d::Degree,gz4d::PositivePeriod> a2(45.0);
    gz4d::Angle<double,gz4d::Degree,gz4d::PositivePeriod> a3(-12.3);
    gz4d::Angle<double,gz4d::Degree,gz4d::PositivePeriod> a4(234.56);
    gz4d::Angle<double,gz4d::Degree,gz4d::PositivePeriod> a5(365.0);
    
    EXPECT_EQ(0.0,a1.value());
    EXPECT_EQ(45.0,a2.value());
    EXPECT_EQ(347.7,a3.value());
    EXPECT_EQ(234.56,a4.value());
    EXPECT_EQ(5.0,a5.value());
    
    EXPECT_EQ(45.0,(a1+a2).value());
    EXPECT_EQ(315.0,(a1-a2).value());
    
    EXPECT_EQ(215.0,(a2-910).value());
    EXPECT_EQ(235.0,(a2+910).value());
    
    EXPECT_EQ(54.56,(a4+180.0).value());
    
    EXPECT_EQ(a1,a2-a2);
}
