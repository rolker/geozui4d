#ifndef GZ4D_KINECT_H
#define GZ4D_KINECT_H

//gz4d_cmake depends ni

#include <gz4d/gz4d.h>
#include <gz4d/Image.h>
#include <gz4d/Point.h>
#include "XnCppWrapper.h"
#include <iostream>
#include <sstream>
#include <gz4d/vector/ios.h>
#include <gz4d/glo/VertexArray.h>

namespace gz4d
{
    class Kinect
    {
        xn::Context _context;
        xn::DepthGenerator _depthGenerator;
        xn::ImageGenerator _imageGenerator;
        xn::DepthMetaData _depthMetaData;
        xn::ImageMetaData _imageMetaData;
        
        XnRGB24Pixel* _imagePixels;
        
        public:
            struct Frame
            {
                ImageL16 depth;
                ImageRGB8 image;
            };

/*            struct PointCloud
            {
                std::vector<Point<float> > points;
                ImageRGB8 image;
                PointCloud(ImageRGB8 const &i):image(i){}
                PointCloud(){}
            };*/
            
            typedef std::vector<gz4d::glo::VertexArray::C3F_V3F> PointCloud;

            Kinect()
            {
                //XnStatus rc = _context.Init();
                XnStatus rc = _context.InitFromXmlFile("/home/roland/kinect.xml");

                if (rc != XN_STATUS_OK)
                {
                    std::cerr << "Open failed in gz4d::Kinect: " << xnGetStatusString(rc) << std::endl;
                    return;
                }

/*                rc = _depthGenerator.Create(_context);
                if (rc != XN_STATUS_OK)
                {
                    std::cerr << "Failed to create depth node in gz4d::Kinect: " << xnGetStatusString(rc) << std::endl;
                }*/
                
                rc = _context.FindExistingNode(XN_NODE_TYPE_DEPTH, _depthGenerator);
                if (rc != XN_STATUS_OK)
                {
                    std::cerr << "Failed to find depth node in gz4d::Kinect: " << xnGetStatusString(rc) << std::endl;
                    return;
                }
                
                rc = _context.FindExistingNode(XN_NODE_TYPE_IMAGE, _imageGenerator);
                if (rc != XN_STATUS_OK)
                {
                    std::cerr << "Failed to find image node in gz4d::Kinect: " << xnGetStatusString(rc) << std::endl;
                    return;
                }
                
                //imageGen.GetAlternativeViewPointCap().SetViewPoint(depthGen);
                _depthGenerator.GetAlternativeViewPointCap().SetViewPoint(_imageGenerator);
                
                _depthGenerator.GetMetaData(_depthMetaData);
                _imageGenerator.GetMetaData(_imageMetaData);
                
                rc =_context.StartGeneratingAll();
                if (rc != XN_STATUS_OK)
                {
                    std::cerr << "Failed starting to generate in gz4d::Kinect: " << xnGetStatusString(rc) << std::endl;
                }
                
            }

            Frame getFrame()
            {
                Frame ret;

                XnStatus rc = XN_STATUS_OK;

                // Read a new frame
                rc = _context.WaitAnyUpdateAll();
                if (rc != XN_STATUS_OK)
                {
                    printf("Read failed in Kinect::pullFrame: %s\n", xnGetStatusString(rc));
                    return ret;
                }
                
                _depthGenerator.GetMetaData(_depthMetaData);
                _imageGenerator.GetMetaData(_imageMetaData);
                
                ret.depth.Resize(_depthMetaData.XRes(),_depthMetaData.YRes());
                for (XnUInt y = 0; y < _depthMetaData.YRes(); ++y)
                {
                    XnUInt fy = _depthMetaData.YRes()-1-y;
                    for (XnUInt x = 0; x < _depthMetaData.XRes(); ++x)
                    {
                        ret.depth.GetRaster().GetRow(fy)[x][0] = _depthMetaData.Data()[y*_depthMetaData.XRes()+x];
                    }
                }

                ret.image.Resize(_imageMetaData.XRes(),_imageMetaData.YRes());
                for (XnUInt y = 0; y < _imageMetaData.YRes(); ++y)
                {
                    XnUInt fy = _imageMetaData.YRes()-1-y;
                    for (XnUInt x = 0; x < _imageMetaData.XRes(); ++x)
                    {
                        ret.image.GetRaster().GetRow(fy)[x][0] = _imageMetaData.RGB24Data()[y*_depthMetaData.XRes()+x].nRed;
                        ret.image.GetRaster().GetRow(fy)[x][1] = _imageMetaData.RGB24Data()[y*_depthMetaData.XRes()+x].nGreen;
                        ret.image.GetRaster().GetRow(fy)[x][2] = _imageMetaData.RGB24Data()[y*_depthMetaData.XRes()+x].nBlue;
                    }
                }
                
                return ret;
            }
            
            PointCloud getPointCloud(float maxRange = 10.0)
            {
                float maxRange_mm = maxRange*1000.0;
                Frame f = getFrame();

                //PointCloud ret(f.image);
                PointCloud ret;
                std::vector<XnPoint3D> points; // points(_depthMetaData.XRes()*_depthMetaData.YRes());
                assert(_depthMetaData.XRes() == _imageMetaData.XRes() && _depthMetaData.XRes() == _imageMetaData.XRes());
                
                //ret.points.resize(points.size());
                for (XnUInt y = 0; y < _depthMetaData.YRes(); ++y)
                {
                    for (XnUInt x = 0; x < _depthMetaData.XRes(); ++x)
                    {
                        float d = f.depth.GetRaster().GetRow(y)[x][0];
                        if(d > 0.0 && d < maxRange_mm)
                        {
                            XnPoint3D xp;
                            xp.X = x;
                            xp.Y = y;
                            xp.Z = d;//f.depth.GetRaster().GetRow(y)[x][0];
                            //points.push_pack(Point<float>(x,y,f.depth.GetRaster().GetRow(y)[x][0]));
                            points.push_back(xp);
                            gz4d::glo::VertexArray::C3F_V3F p;
                            gz4d::Color c(f.image.GetRaster().GetRow(y)[x]);
                            p.r = c[0];
                            p.g = c[1];
                            p.b = c[2];
                            ret.push_back(p);
                        }
                    }
                }
                
                if(!points.empty())
                {
                    std::vector<XnPoint3D> rwPoints(points.size());
                    _depthGenerator.ConvertProjectiveToRealWorld(points.size(), &points.front(),&rwPoints.front());
                    for(int i = 0; i < rwPoints.size(); ++i)
                    {
                        ret[i].x = rwPoints[i].X/1000.0;
                        ret[i].y = -rwPoints[i].Y/1000.0;
                        ret[i].z = -rwPoints[i].Z/1000.0;
                    }

//                 for (XnUInt y = 0; y < _depthMetaData.YRes(); y+=10)
//                 {
//                     for (XnUInt x = 0; x < _depthMetaData.XRes(); x+=10)
//                     {
//                         std::cerr << points[y*_depthMetaData.XRes()+x] << "\t" << ret.points[y*_depthMetaData.XRes()+x] << std::endl;
//                     }
//                 }
                }
                return ret;
            }
    };
}

#endif
