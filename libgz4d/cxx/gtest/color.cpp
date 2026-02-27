#include<gtest/gtest.h>

#include <sstream>

#include <../include/gz4d/Color.h>
#include <../include/gz4d/vector/ios.h>

#include <boost/property_tree/xml_parser.hpp>

TEST(color,XML)
{
    std::istringstream xml1("<Color r=\".25\" g=\"0.5\" b=\"0.750\"/>\n");
    boost::property_tree::ptree pt1;
    read_xml(xml1,pt1);
    
    gz4d::Color c1(pt1.begin()->second);
    
    gz4d::Color t1(.25,.5,.75);
    
    EXPECT_EQ(t1,c1);
}

    