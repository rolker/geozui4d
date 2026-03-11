// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gtest/gtest.h>

#include <../include/gz4d/Matrix.h>
#include <../include/gz4d/matrix/operations.h>

TEST(matrix,inverse)
{
    gz4d::Matrix<double,4,4> m1;
    m1(0,0) = 1.0;
    m1(1,2) = 1.0;
    m1(2,1) = 1.0;
    m1(3,3) = 1.0;
    
    EXPECT_EQ(-1.0,determinant(m1));
    
    EXPECT_EQ(m1,inverse(m1));
}

    