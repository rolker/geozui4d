#ifndef GZ4D_GEOCAM_PROJECTEDIMAGE_H
#define GZ4D_GEOCAM_PROJECTEDIMAGE_H

#include "Image.h"
#include "Platform.h"
#include <gz4d/Pixel.h>
#include <gz4d/geo/Image.h>
#include <gz4d/gdal/saveGeotiff.h>
#include <gz4d/image/io.h>
#include <gz4d/cv/Histogram.h>
#include <gz4d/geo/LocalENU.h>
#include <gz4d/ios.h>
#include <gz4d/vector/ios.h>
#include <gz4d/nav/ios.h>


namespace gz4d
{
    namespace geocam
    {
        class ProjectedImage: public Image
        {
            nav::Fix::Ptr _nav;
            Camera::Ptr _camera;
            geo::Projection _projection;
            
        public:
            ProjectedImage(){}
            
            ProjectedImage(Image const &i, Platform const &p, geo::Projection const &proj):Image(i),_projection(proj)
            {
                _camera = p.getCamera(_camera_label);
                _nav = p.getFix(_timestamp);
            }
            
            Box2d getBoundingBox(double maxDistance)
            {
                Vector<double,2> corner(maxDistance,maxDistance);
                if(_projection)
                {
                    Point<double> projectedPosition = _projection(*_nav);
                    return Box2d(Vector<double,2>(projectedPosition,0)-corner,Vector<double,2>(projectedPosition,0)+corner);
                }
                
                Box2d ret(-corner,corner);
                geo::LocalENU<> lenu = geo::Point<double,geo::WGS84::LatLon>(*_nav);
                
                return lenu.toLonLatBox(ret);
            }
            
            nav::Fix const &getNav() const
            {
                return *_nav;
            }

            nav::Fix::Ptr getNavPtr() const
            {
                return _nav;
            }
            
            Camera const &getCamera() const
            {
                return *_camera;
            }
            
            Camera &getCamera()
            {
                return *_camera;
            }
            
            Point<double> project(Vector<double,2> const &img_point, double altitude = 0.0)
            {
                
            }
            
            geo::Image<RGBA8> generate(double maxDistance, Vector<double,2> const &resolution, bool weighted=false)
            {
                if(!_nav->IsComplete())
                    return geo::Image<RGBA8>();
                
                Box2d bbox = getBoundingBox(maxDistance);
                
                Vector <double,2> outSize(maxDistance,maxDistance);
                //outSize *= 2.0;
                outSize /= resolution;
                
                Vector <std::size_t,2> outSizei = ceil(outSize);
                outSizei *= 2;
                
                if(!_projection)
                {
                    geo::LocalENU<> lenu = geo::Point<double,geo::WGS84::LatLon>(*_nav);
                    geo::Point< double, geo::WGS84::LatLon > pll(lenu.toECEF(Vector<double,3>(resolution,0)));
                    Vector<double,2> dpll(pll[1]-_nav->longitude.value(),pll[0]-_nav->latitude.value());
                    Vector<double,2> cll(_nav->longitude.value(),_nav->latitude.value());
                    bbox = Box2d(cll-dpll*outSizei/2,cll+dpll*outSizei/2);
                    //bbox.setMax(cll+dpll*outSizei/2);
                    //bbox.setMin(cll-dpll*outSizei/2);
                }
                
                //Vector <double,2> border = outSizei - outSize;
                
                //border /= outSize; // border factor relative to original size
                //border /= 2.0;
                
                //geo::Image<RGBA8> outImage(ImageRGBA8(outSizei,Pixel<RGBA8>(Color(0,0,0,0))),_projection,Box2d(bbox.getMin()-(bbox.getSizes()*border),bbox.getMax()+(bbox.getSizes()*border)));
                geo::Image<RGBA8> outImage(ImageRGBA8(outSizei,Pixel<RGBA8>(Color(0,0,0,0))),_projection,bbox);
                
                gz4d::ImageRGB8 inImage;
                inImage.Load(_filename);
                
//                 ImageL8 bw(inImage.GetRaster());
//                 ImageL8 equalized = cv::EqualizeHistogram(bw);

                gz4d::geo::LocalENU<> lenu = (gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(*_nav));
                
                double md2 = maxDistance*maxDistance;
                
                
                for(int x = 0; x < outImage.width(); ++x)
                {
                    for(int y = 0; y < outImage.height(); ++y)
                    {
                        geo::Position<double> p = outImage.getPixelLatLonPosition(x,y);
                        geo::Point<double,geo::WGS84::LatLon> p3d(p.latitude.value(),p.longitude.value(),0);
                        geo::Point<double,geo::WGS84::ECEF> pecef(p3d);
                        gz4d::Point< double > localPoint = lenu.toLocal(pecef);
                        
                        double d2 = gz4d::norm2(localPoint);
                        if(d2 > 3600 && d2 < md2)
                        {
                            gz4d::Vector< int, 3 > uvd = _camera->relativePointToImage(localPoint,_nav);
                            if(uvd[0]>=0 && uvd[0] < inImage.width() && uvd[1]>=0 && uvd[1] < inImage.height() && uvd[2] > 0.0)
                            {
                                if(weighted)
                                {
                                    Color c = inImage.GetRaster().GetRow(uvd[1])[uvd[0]];
                                    double w = 1.0-sqrt(d2)/maxDistance;
                                    w *= cos(M_PI*(-.5+(uvd[0]/double(inImage.width()))));
                                    w *= cos(M_PI*(-.5+(uvd[1]/double(inImage.height()))));
                                    c[3] = w;
                                    outImage.GetRaster().GetRow(y)[x] = c;
                                }
                                else
                                    outImage.GetRaster().GetRow(y)[x] = inImage.GetRaster().GetRow(uvd[1])[uvd[0]];
                            }
//                             else
//                             {
//                                 float r = .5+.05*uvd[0]/float(outImage.width());
//                                 float g = .5+.05*uvd[1]/float(outImage.height());
//                                 float b = .5+uvd[2]/maxDistance;
//                                 outImage.GetRaster().GetRow(y)[x] = Color(r,g,b);
//                             }
                        }                            
                    }
                }
                //gdal::saveGeotiff((boost::filesystem::path("debug")/boost::filesystem::path(_filename).filename().replace_extension(".tif")).string(),outImage);
                outImage = outImage.strip(gz4d::Pixel<gz4d::RGBA8>(gz4d::Color(0,0,0,0)));
                
                
                return outImage;
            }
        };
    }
}
            

#endif
