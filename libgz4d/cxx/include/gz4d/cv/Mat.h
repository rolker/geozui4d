// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_CV_MAT_H
#define GZ4D_CV_MAT_H

//gz4d_cmake depends opencv

#include <gz4d/gz4d.h>
#include <opencv2/opencv.hpp>
#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>

namespace gz4d
{
    namespace cv
    {

        ::cv::Mat inline toMat(Raster<Pixel<RGB8> > const &r)
        {
            ::cv::Mat ret(r.GetRows(),r.GetColumns(),CV_8UC3);
            for(int row = 0; row < r.GetRows(); ++row)
            {
                int drow = r.GetRows()-1-row;
                for(int col = 0; col < r.GetColumns(); ++col)
                {
                    ret.at< ::cv::Vec3b>(drow,col).val[0] = r.GetRow(row)[col][2];
                    ret.at< ::cv::Vec3b>(drow,col).val[1] = r.GetRow(row)[col][1];
                    ret.at< ::cv::Vec3b>(drow,col).val[2] = r.GetRow(row)[col][0];
                }
            }
            return ret;
        }
        
        template <typename T> struct Mat
        {
            typename Raster<T>::Ptr raster;
            ::cv::Mat mat;
            Mat(typename Raster<T>::Ptr r);
        };

        template <> inline Mat<float>::Mat(Raster<float>::Ptr r):raster(r),mat(r->GetRows(),r->GetColumns(),CV_32F,r->raw()){}
        template <> inline Mat< Vector<float,2> >::Mat(Raster<Vector<float,2> >::Ptr r):raster(r),mat(r->GetRows(),r->GetColumns(),CV_32FC2,r->raw()){}
        
        template <> inline Mat<Pixel<RGB8> >::Mat(Raster<Pixel<RGB8> >::Ptr r):raster(r),mat(r->GetRows(),r->GetColumns(),CV_8UC3,r->raw()){}
        template <> inline Mat<Pixel<RGBA8> >::Mat(Raster<Pixel<RGBA8> >::Ptr r):raster(r),mat(r->GetRows(),r->GetColumns(),CV_8UC4,r->raw()){}

        template <> inline Mat<Pixel<L8> >::Mat(Raster<Pixel<L8> >::Ptr r):raster(r),mat(r->GetRows(),r->GetColumns(),CV_8UC1,r->raw()){}
        template <> inline Mat<Pixel<L32> >::Mat(Raster<Pixel<L32> >::Ptr r):raster(r),mat(r->GetRows(),r->GetColumns(),CV_32FC1,r->raw()){}
        
    }
}

#endif
