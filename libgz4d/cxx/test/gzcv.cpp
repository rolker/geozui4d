// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/cv/Mat.h>
#include <gz4d/Image.h>
#include <gz4d/image/io.h>

int main(int argc, char *argv[])
{
    gz4d::ImageRGB8 image;
    image.Load(argv[1]);
    //gz4d::cv::Mat<gz4d::Pixel<gz4d::RGB8> > m(image.GetRasterPtr());

    cv::Mat m = gz4d::cv::toMat(image.GetRaster());

    cv::namedWindow("gzcv");
    cv::imshow("gzcv",m);
    cv::waitKey();
    return 0;
}