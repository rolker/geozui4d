#ifndef GZ4D_GEOCAM_MAP_H
#define GZ4D_GEOCAM_MAP_H

#include "ProjectedImage.h"
#include <gz4d/geo/Projection.h>
#include <gz4d/geo/LocalENU.h>
#include <gz4d/geo/WGS84.h>
#include <gz4d/av.h>

namespace gz4d
{
    namespace geocam
    {
        class Map
        {
            geo::Projection _projection;
            Vector<double,2> _resolution;
            double _range;
            Time _begin_time;
            Time _end_time;
            Time _target_time;
            
            Platform::Ptr _platform;
            ImageSets _images;
            
        public:
            Map(Platform::Ptr platform, ImageSets images, geo::Projection const &proj):_projection(proj),_resolution(1.0),_platform(platform),_images(images)
            {
            }
            
            void setResolution(double r)
            {
                _resolution = Vector<double,2>(r);
            }
            
            void setResolution(Vector<double,2> const &r)
            {
                _resolution = r;
            }
            
            Vector<double,2> const &getResolution() const
            {
                return _resolution;
            }
            
            void setRange(double r)
            {
                _range = r;
            }
            
            void animate(std::string const &outfile, nav::Track::Ptr nav, std::size_t width, std::size_t height)
            {
                TimeSequence<ProjectedImage> pimages;
                
                for(std::map< std::string, ImageSet::Ptr >::iterator is = _images.begin(); is != _images.end(); ++is)
                {
                    for( TimeSequence< Image >::iterator i = is->second->getImages().begin(); i != is->second->getImages().end(); ++i)
                    {
                        ProjectedImage pi(i->second,*_platform,_projection);
                        pimages[i->first]=pi;
                    }
                }
                
                Vector<std::size_t,2> outSize(width,height);
                
                gz4d::AVWriter avw;
                avw.SetSize(outSize);
                avw.SetFPS(30);
                avw.Open(outfile);
                
                Box2d frame_bbox(Vector<double,2>(),Vector<double,2>(width,height)*_resolution);
                frame_bbox -= frame_bbox.getCenter();

                geo::Image<RGBA8> ret(ImageRGBA8(outSize,Pixel<RGBA8>(Color(0,0,0,0))),_projection,frame_bbox);
                
                Time t = pimages.begin()->first;
                nav::Fix::Ptr fix = (*nav)(t);
                TimeSequence< ProjectedImage >::iterator i = pimages.begin();
                
                if(_projection)
                    ret.getBoundingBox() += Vector< double, 2 >(_projection(*fix),0);
                else
                    ret.setLatLonBoundingBox(geo::Point<double,geo::WGS84::LatLon>(*fix),_resolution);
                
                while(i != pimages.end())
                {
                    while(i != pimages.end() && t >= i->first)
                    {
                        std::cout << i->second.getFilename() << std::endl;
                        
                        geo::Image< RGBA8 > pi = i->second.generate(_range,_resolution,false);
                        if(!pi.empty())
                        {
                            //pi.Save("debug/projected.png");
                            //gdal::saveGeotiff("debug/projected.tif",pi);
                            ret.blend(pi);
                        }
                        //ret.Save("debug/frame.png");
                        //gdal::saveGeotiff("debug/frame.tif",ret);
                        ++i;
                    }
                    while(i != pimages.end() && t < i->first)
                    {
                        gz4d::ImageYUV8 yuv(ret.GetRaster());
                        avw.AddFrame(yuv);
                        
                        Time nt = t + seconds(2);
                        nav::Fix::Ptr next_fix = (*nav)(nt);
                        
                        gz4d::Vector< double, 2 > shift;
                        if(_projection)
                            shift= ret.getBoundingBox().getCenter() - gz4d::Vector< double, 2 >(_projection(*next_fix),0);
                        else
                            shift= ret.getBoundingBox().getCenter() - gz4d::Vector< double, 2 >(next_fix->longitude.value(), next_fix->latitude.value());
                        gz4d::Vector< int, 2 > shifti( shift /= ret.getPixelSizes());
                        
                        ImageRGBA8 new_ret(outSize,Pixel<RGBA8>(Color(0,0,0,0)));
                        for(int y = 0; y < new_ret.height(); ++y)
                        {
                            int ny = y+shifti[1];
                            if(ny >=0 && ny < new_ret.height())
                            {
                                Pixel< RGBA8 >* row = ret.GetRaster().GetRow(y);
                                Pixel< RGBA8 >* nrow = new_ret.GetRaster().GetRow(ny);
                                for(int x = 0; x < new_ret.width(); ++x)
                                {
                                    int nx = x+shifti[0];
                                    if(nx >= 0 && nx < new_ret.width())
                                        nrow[nx] = row[x];
                                }
                            }
                        }
                        
                        Box2d new_bbox;
                        if(_projection)
                        {
                            new_bbox = ret.getBoundingBox()- (_resolution*shifti);
                        }
                        else
                        {
                            new_bbox = ret.getBoundingBox()- (ret.getPixelSizes()*shifti);
                            //gz4d::Vector< double, 2 > new_c = ret.getBoundingBox().getCenter()-(shifti*ret.getPixelSizes());
                            //gz4d::geo::LocalENU<> lenu = (gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon>(new_c[1],new_c[0],0.0));
                            //new_bbox = lenu.toLonLatBox(frame_bbox);
                        }
                        ret = geo::Image<RGBA8>(new_ret,_projection,new_bbox);
                        t = nt;
                        fix = next_fix;
                    }
                    
                }
                
                gz4d::ImageYUV8 yuv(ret.GetRaster());
                for(int i = 0; i < 150; ++i)
                    avw.AddFrame(yuv);
            }
            
            geo::Image<RGBA8> generate()
            {
                TimeSequence<ProjectedImage> pimages;
                
                Box<Vector<double,2> > bbox;
                
                for(std::map< std::string, ImageSet::Ptr >::iterator is = _images.begin(); is != _images.end(); ++is)
                {
                    for( TimeSequence< Image >::iterator i = is->second->getImages().begin(); i != is->second->getImages().end(); ++i)
                    {
                        ProjectedImage pi(i->second,*_platform,_projection);
                        pimages[i->first]=pi;
                        bbox.expand(pi.getBoundingBox(_range));
                    }
                }
                
                gz4d::Vector< double, 2 > pmin = floor(bbox.getMin()/_resolution);
                gz4d::Vector< double, 2 > pmax = ceil(bbox.getMax()/_resolution);                
                
                bbox = Box2d(pmin*_resolution,pmax*_resolution);
                
                Vector<std::size_t,2> outSize = ceil(bbox.getSizes()/_resolution);
                
                //gz4d::Raster<gz4d::Vector<float,4> > outData(outSize ,gz4d::Vector<float,4>(0.0,0.0,0.0,0.0));
                ImageRGBA8 ret(outSize,Pixel<RGBA8>(Color(0,0,0,0)));
                
//                 double vidScale = std::max(outSize[0]/1920.0,outSize[1]/1080.0);
//                 Vector<std::size_t,2> vidSize = outSize/vidScale;
                
//                 gz4d::AVWriter avw;
//                 avw.SetSize(vidSize);
//                 avw.SetFPS(5);
//                 avw.Open("debug/out.avi");
                
                
                gz4d::Point< double > basePoint = Vector<double,3>(bbox.getMin(),0);
                
                int frame = 0;
                
                for(TimeSequence< ProjectedImage >::iterator i = pimages.begin(); i != pimages.end(); ++i)
                {
                    std::cout << i->second.getFilename() << std::endl;
                    geo::Image< RGBA8 > pi = i->second.generate(_range,_resolution,false);
                    if(!pi.empty())
                    {
                        gz4d::Vector< int, 2 > offset = (pi.getBoundingBox().getMin()-bbox.getMin())/_resolution;
                        for(int y = 0; y < pi.height(); ++y)
                        {
                            Pixel< RGBA8 >* src_row = pi.GetRaster().GetRow(y);
                            //gz4d::Vector< float, 4 >* dst_row = outData.GetRow(y+offset[1]);
                            Pixel< RGBA8 >* dst_row = ret.GetRaster().GetRow(y+offset[1]);
                            for(int x = 0; x < pi.width(); ++x)
                                if(src_row[x][3])
                                {
    //                                 Color c = src_row[x];
    //                                 dst_row[x+offset[0]][0] += c[0]*c[3];
    //                                 dst_row[x+offset[0]][1] += c[1]*c[3];
    //                                 dst_row[x+offset[0]][2] += c[2]*c[3];
    //                                 dst_row[x+offset[0]][3] += c[3];
                                    dst_row[x+offset[0]] = src_row[x];
                                }
                        }
    //                     std::string debug_fname = boost::lexical_cast<std::string>(frame);
    //                     while(debug_fname.size() < 4)
    //                         debug_fname = "0"+debug_fname;
    //                     debug_fname = "frames/frame"+debug_fname+".tif";
    //                     ret.Save(debug_fname);
                        ++frame;
                    }
//                     gz4d::ImageYUV8 yuv(gz4d::ImageRGBA8(vidSize[0],vidSize[1],ret).GetRaster());
//                     avw.AddFrame(yuv);
                }
//                 for(int y = 0; y < outData.GetRows(); ++y)
//                 {
//                     gz4d::Vector< float, 4 >* src_row = outData.GetRow(y);
//                     Pixel< RGBA8 >* dst_row = ret.GetRaster().GetRow(y);
//                     for(int x = 0; x < outData.GetColumns(); ++x)
//                         if(src_row[x][3])
//                         {
//                             Color c;
//                             c[0] = src_row[x][0]/src_row[x][3];
//                             c[1] = src_row[x][1]/src_row[x][3];
//                             c[2] = src_row[x][2]/src_row[x][3];
//                             c[3] = 1.0;
//                             dst_row[x] = c;
//                         }
//                 }
                
                return geo::Image<RGBA8>(ret,_projection,bbox).strip(Pixel<RGBA8>(Color(0,0,0,0)));
            }
            
        };
    }
}

#endif
