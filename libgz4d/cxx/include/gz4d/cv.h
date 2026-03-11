// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_CV_H
#define GZ4D_CV_H

//gz4d_cmake depends opencv

#include <opencv2/opencv.hpp>
#include "cv/Mat.h"
#include "Vector.h"

namespace gz4d
{
    namespace cv
    {
        template<typename T> class Point_: public ::cv::Point_<T>
        {
        public:
            Point_(){}
            Point_(Point_ const &o): ::cv::Point_<T>(o){}
            Point_(::cv::Point_<T> const &o): ::cv::Point_<T>(o){}
            
            operator Vector<T,2> const &() const 
            {
                return *reinterpret_cast<const Vector<T,2>*>(this);
            }
        };
        
        typedef Point_<double> Point2d;
    }
}

#endif
