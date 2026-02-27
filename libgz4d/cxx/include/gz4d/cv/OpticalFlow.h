#ifndef GZ4D_CV_OPTICALFLOW_H
#define GZ4D_CV_OPTICALFLOW_H

//gz4d_cmake depends opencv

#include <gz4d/gz4d.h>
#include "../cv.h"
#include <gz4d/Image.h>

namespace gz4d
{
    namespace cv
    {
        struct OpticalFlow
        {
            ::cv::Mat2f features;
            ::cv::Mat moved_features;
            ::cv::Mat status;
            ::cv::Mat errors;
            ::cv::Mat transform;
            gz4d::Vector<float,2> avg;
            
            OpticalFlow(gz4d::ImageL8 &i1, gz4d::ImageL8 &i2)
            {
                gz4d::cv::Mat<gz4d::Pixel<gz4d::L8> > cvimg1(i1.GetRasterPtr());
                gz4d::cv::Mat<gz4d::Pixel<gz4d::L8> > cvimg2(i2.GetRasterPtr());
                
//                 ::cv::goodFeaturesToTrack(cvimg1.mat,features,250,0.05,3);
//                 
//                 if(*features.size)
//                 {
//                     ::cv::calcOpticalFlowPyrLK(cvimg1.mat,cvimg2.mat,features,moved_features,status,errors);
//                 
//                     float total_weights = 0.0;
//                     
//                     for(int i = 0; i < *(moved_features.size); ++i)
//                     {
//                         //std::cout << status.at<bool>(i) << "\t" << errors.at<float>(i) << std::endl;
//                         if(status.at<bool>(i))
//                         {
//                             avg += (moved_features.at<gz4d::Vector<float,2> >(i) - features.at<gz4d::Vector<float,2> >(i))*1.0/errors.at<float>(i);
//                             total_weights += 1.0/errors.at<float>(i);
//                         }
//                     }
//                     avg /= total_weights;
//                 }
                
                transform = ::cv::estimateRigidTransform(cvimg1.mat,cvimg2.mat,false);
            }
        };
    }
}

#endif
