#include<gtest/gtest.h>

#include "../include/gz4d/geocam/Camera.h"

TEST(geocam,Camera)
{
    gz4d::geocam::Camera c("test-camera");
    c.setImageSize(200,100);
    c.setCX(100);
    c.setCY(50);
    c.setFX(250);
    c.setFY(250);
    
    gz4d::Point<double> camera_point(0.0,0.0,1.0);
    gz4d::Point<double> ground_point = c.getCameraToGround()*camera_point;
    
    gz4d::Point<double> expected_ground_point(0.0,1.0,0.0);

    EXPECT_EQ(expected_ground_point,ground_point);
    
    gz4d::Vector<double,3> v1 = c.imageToVector(0,0);
    gz4d::Vector<double,3> expected_v1(-100/250.0,1.0,-50/250.0);
    EXPECT_EQ(expected_v1,v1);
    
    gz4d::Point<double> offset(0.0,0.0,10.0);
    c.setOffset(offset);
    v1 = c.imageToVector(0,0);
    EXPECT_NEAR(expected_v1[0],v1[0],1e-12);
    EXPECT_NEAR(expected_v1[1],v1[1],1e-12);
    EXPECT_NEAR(expected_v1[2],v1[2],1e-12);
    
    v1 = c.imageToPlatformPoint(0,0);
    expected_v1 += offset;
    EXPECT_NEAR(expected_v1[0],v1[0],1e-12);
    EXPECT_NEAR(expected_v1[1],v1[1],1e-12);
    EXPECT_NEAR(expected_v1[2],v1[2],1e-12);
}

