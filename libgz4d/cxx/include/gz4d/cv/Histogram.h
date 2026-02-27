#ifndef GZ4D_CV_HISTOGRAM_H
#define GZ4D_CV_HISTOGRAM_H

//gz4d_cmake depends opencv

#include "Mat.h"
#include <gz4d/Image.h>

namespace gz4d
{
    namespace cv
    {
        inline ImageL8 EqualizeHistogram(ImageL8 &img)
        {
            Mat<Pixel<L8> > in_mat(img.GetRasterPtr());
            ImageL8 ret;
            ret.Resize(img.getSizes());
            Mat<Pixel<L8> > out_mat(ret.GetRasterPtr());
            
            ::cv::equalizeHist(in_mat.mat,out_mat.mat);
            
            return ret;
            
        }
    }
}

#endif
