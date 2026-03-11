// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gtest/gtest.h>

#include "../include/gz4d/time.h"
#include "../include/gz4d/time/ios.h"
#include <boost/lexical_cast.hpp>

TEST(Time,ios)
{
    gz4d::Time t = boost::lexical_cast<gz4d::Time>("2012-08-31_12.45.32.132");
    
    std::string ts = boost::lexical_cast<std::string>(t);
    
    EXPECT_EQ("2012-08-31_12.45.32.132",ts);
}
