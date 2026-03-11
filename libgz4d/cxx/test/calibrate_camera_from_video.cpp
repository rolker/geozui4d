// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/cv/Mat.h>
#include <gz4d/av/AVReader.h>
#include <iostream>


class VideoReader
{
    bool done;
    boost::mutex doneMutex;
    boost::thread vThread;

public:
    VideoReader(gz4d::AVReader::Ptr avReader, gz4d::AVReader::VideoStream::Ptr videoStream):done(false)
    {
        vThread = std::move(boost::thread(boost::bind(&VideoReader::videoThread,this,avReader,videoStream)));
    }
    
    void videoThread(gz4d::AVReader::Ptr avReader, gz4d::AVReader::VideoStream::Ptr vs)
    {
        while(!boost::this_thread::interruption_requested() && avReader->ReadFrame())
        {
            boost::this_thread::yield();
        }
        boost::mutex::scoped_lock lock(doneMutex);
        done = true;
    }

    bool isDone()
    {
        boost::mutex::scoped_lock lock(doneMutex);
        return done;
    }
};

int main(int argc, char * argv[])
{
    if(argc == 1)
    {
        std::cerr << "usage: calibrate_camera_from_video infile [start_time [end_time]]" << std::endl;
        return 1;
    }

    gz4d::time::Duration startTime(0.0);
    if(argc >= 3)
        startTime = gz4d::time::Duration(atof(argv[2]));

    
    gz4d::AVReader::Ptr avr(new gz4d::AVReader);
    avr->Open(argv[1]);

    if(avr->GetVideoStreams().empty())
    {
        std::cerr << "No video streams found!" << std::endl;
        return 1;
    }

    gz4d::AVReader::VideoStream::Ptr vs = avr->GetVideoStreams()[0];

    gz4d::time::Duration endTime;
    if(argc == 4)
        endTime = gz4d::time::Duration(atof(argv[3]));
    else
        endTime = vs->GetDuration();
    
    //vs->Seek(startTime);
    
    gz4d::AVReader::VideoStream::output_queue_type vq = vs->NewOutQueue(10);
    VideoReader reader(avr,vs);
    
    cv::namedWindow("calibrate");

    cv::Size2i pattern_size(8,6);

    std::vector<std::vector<cv::Point2f> > all_corners;
    
    while(!reader.isDone())
    {
        while(vq->empty())
            boost::this_thread::yield();
        gz4d::AVReader::VideoFrame f = vq->pop();
        if(f.presentationTime > endTime)
            break;
        std::cerr << f.presentationTime << " of " << vs->GetDuration() << " (end at " << endTime << ")" << std::endl;
        gz4d::ImageRGB8 img(f.frame.GetRaster());
        cv::Mat m = gz4d::cv::toMat(img.GetRaster());
        //std::cerr << m.size[0] << "x" << m.size[1] << std::endl;
        std::vector<cv::Point2f> corners;
        if(cv::findChessboardCorners(m,pattern_size,corners,cv::CALIB_CB_ADAPTIVE_THRESH|cv::CALIB_CB_NORMALIZE_IMAGE))
        {
            std::cerr << "found!" << std::endl;
            all_corners.push_back(corners);
        }
        cv::resize(m,m, cv::Size2i( m.size[1]/3,m.size[0]/3));
        cv::imshow("calibrate",m);
        if(cv::waitKey(100) != -1)
            break;
    }

    std::cerr << all_corners.size() << " corner sets found" << std::endl;
    
    return 0;
}
