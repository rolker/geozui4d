// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_IMAGE_H
#define GZ4D_GEO_IMAGE_H

#include <gz4d/gz4d.h>
#include <gz4d/Image.h>
#include "Projection.h"
#include <gz4d/Box.h>
#include "LocalENU.h"
#include <gz4d/vector/ios.h>

namespace gz4d
{
    namespace geo
    {
        template<typename PF> class Image: public TypedImage<Pixel<PF> >
        {
            Projection _projection;
            Box2d _boundingBox;

            
        public:
            Image(){}
            
            Image(Projection const &p, Box2d const &b):_projection(p),_boundingBox(b)
            {
            }
            
            Image(TypedImage<Pixel<PF> > const &i, Projection const &p, Box2d const &b):TypedImage<Pixel<PF> >(i), _projection(p),_boundingBox(b)
            {
            }
            
            void setProjection(Projection const &p)
            {
                _projection = p;
            }
            
            Projection const &getProjection() const
            {
                return _projection;
            }
            
            Box2d const & setBoundingBox(Box2d const &b)
            {
                _boundingBox = b;
                return _boundingBox;
            }

            /// Creates a bounding box in geographic coordinates centered on supplied coordinates.
            /// The corners are derived from calculating the coordinates half the image size away from the center scaled by meters_per_pixel.
            Box2d const &setLatLonBoundingBox(const Point<double,WGS84::LatLon> &center, const Vector<double,2> meters_per_pixel)
            {
                _projection = Projection();
                
                Vector<double,2> half = TypedImage<Pixel<PF> >::getSizes();
                half *= meters_per_pixel/2.0;
                Box2d ret(-half,half);

                LocalENU<> lenu = center;
                
                geo::Point< double, geo::WGS84::LatLon > pll(lenu.toECEF(Vector<double,3>(meters_per_pixel,0)));
                Vector<double,2> dpll(pll[1]-center[1],pll[0]-center[0]);
                Vector<double,2> cll(center[1],center[0]);
                half = TypedImage<Pixel<PF> >::getSizes();
                half /= 2;
                return setBoundingBox(Box2d(cll-dpll*half,cll+dpll*half));
                
                
                return setBoundingBox(lenu.toLonLatBox(ret));
            }
            
            Vector<double,2> getPixelProjectedPosition(double column, double row) const
            {
                return _boundingBox.getMin() + (getPixelSizes()*Vector<double,2>(column,row));
            }
            
            Position<double> getPixelLatLonPosition(double column, double row) const
            {
                Vector<double,2> ret = getPixelProjectedPosition(column, row);
                
                if(_projection)
                    return _projection.inverse(ret);
                return Position<double>(ret[1],ret[0]);
            }
            
            Vector<double,2> getIndexFromProjectedPosition(const Vector<double,2> &pp) const
            {
//                 std::cout << "\tgetIndexFromProjectedPosition: pp - bbox min: " << pp-_boundingBox.getMin() << std::endl;
//                 std::cout << "\tgetIndexFromProjectedPosition: pixel sizes: " << getPixelSizes() << std::endl;
//                 std::cout << "\tgetIndexFromProjectedPosition: (pp - min)/sizes: " << (pp-_boundingBox.getMin())/getPixelSizes() << std::endl;
                return (pp-_boundingBox.getMin())/getPixelSizes();
            }
            
            Vector<double,2> getIndexFromLatLonPosition(Position<double> p) const
            {
                if(_projection)
                {
                    gz4d::Point< double > pp = _projection(p);
                    return getIndexFromProjectedPosition(Vector<double,2>(pp,0));
                }
                return getIndexFromProjectedPosition(Vector<double,2>(p.longitude,p.latitude));
            }

            Image<PF> &blend(const Image<PF> &other)
            {
                if(_projection == other._projection)
                {
                    Box2d overlap_box = _boundingBox&other._boundingBox;
                    if(!overlap_box.empty())
                    {
//                         std::cout << "\tblend this bbox: " << _boundingBox.getMin() << " - " << _boundingBox.getMax() << std::endl;
//                         std::cout << "\tblend other bbox: " << other._boundingBox.getMin() << " - " << other._boundingBox.getMax() << std::endl;
//                         std::cout << "\tblend overlap bbox: " << overlap_box.getMin() << " - " << overlap_box.getMax() << std::endl;
//                         std::cout << "\t\tsizes: " << overlap_box.getSizes() << " pixel sizes: " << getPixelSizes() << std::endl;
//                         std::cout << "\t\tindecies: " << getIndexFromProjectedPosition(overlap_box.getMin()) << " pixel sizes: " << getIndexFromProjectedPosition(overlap_box.getMax()) << std::endl;
                        gz4d::Vector< double, 2 > start = floor(getIndexFromProjectedPosition(overlap_box.getMin()));
                        gz4d::Vector< double, 2 > finish = ceil(getIndexFromProjectedPosition(overlap_box.getMax()));
//                         std::cout << "\t\tstart: " << start << " finish: " << finish << std::endl;
                        for(int y=start[1]; y < finish[1] && y < TypedImage<Pixel<PF> >::height(); ++y)
                            for(int x=start[0]; x < finish[0] && x < TypedImage<Pixel<PF> >::width(); ++x)
                            {
                                gz4d::Vector< double, 2 > pp = getPixelProjectedPosition(x,y);
                                gz4d::Vector< double, 2 > oi = other.getIndexFromProjectedPosition(pp);
                                Color c = TypedImage<Pixel<PF> >::raster->GetRow(y)[x];
                                Color oc = other.getInterpolated(oi,Color(0,0,0,0));
                                TypedImage<Pixel<PF> >::raster->GetRow(y)[x] = c.Blend(oc);
                            }
                    }
                    
                }
                
                return *this;
            }
            
            Box2d const &getBoundingBox() const
            {
                return _boundingBox;
            }
            
            Box2d &getBoundingBox()
            {
                return _boundingBox;
            }
            
            Vector<double,2> getPixelSizes() const
            {
                return _boundingBox.getSizes()/Vector<double,2>(this->getSizes());
            }
            
            Image subImage(Box<Vector<std::size_t,2> > const &box) const
            {
                Vector<double,2> resolution = getPixelSizes();
                Box2d newbox = Box2d(_boundingBox.getMin()+(resolution*box.getMin()),_boundingBox.getMin()+ (resolution*box.getMax()));
                typename Raster<Pixel<PF> >::Ptr subraster = this->raster->subRaster(box);
                return Image(TypedImage<Pixel<PF> >(subraster),_projection,newbox);
            }
            
            Image strip(Pixel<PF> const &blank) const
            {
                Box<Vector<std::size_t,2> > borders = this->raster->findBorders(blank);
                return subImage(borders);
            }
            
            Image nonNullRectangle(Pixel<PF> const &blank=Pixel<PF>(Color(0,0,0,0))) const
            {
                Box<Vector<std::size_t,2> > rect = this->raster->findNonNullBox(blank,true);
                return subImage(rect);
            }

            std::pair<Image,Image> nonNullRectangleOverlap(Image const &other, Pixel<PF> const &blank=Pixel<PF>(Color(0,0,0,0))) const
            {
                std::pair< Image, Image > o = overlap(other);
                
                Box<Vector<std::size_t,2> > rect = o.first.raster->findNonNullBox(blank,*o.second.raster);
                
                return std::pair<Image,Image>(o.first.subImage(rect),o.second.subImage(rect));
            }
            
            
            std::pair<Image,Image> overlap(Image const &other) const
            {
                Box2d bbox = _boundingBox&other.getBoundingBox();
                if(bbox.empty())
                    return std::pair<Image,Image>();
                
                gz4d::Vector< std::size_t, 2 > newmin = (bbox.getMin()-_boundingBox.getMin())/getPixelSizes();
                gz4d::Vector< std::size_t, 2 > newmax = (bbox.getMax()-_boundingBox.getMin())/getPixelSizes();
                Box<Vector<std::size_t,2> > nbbox(newmin,newmax);
                
                gz4d::Vector< std::size_t, 2 > onewmin = (bbox.getMin()-other.getBoundingBox().getMin())/(other.getSizes()/other.getSizes());
                gz4d::Vector< std::size_t, 2 > onewmax = onewmin + nbbox.getSizes();
                Box<Vector<std::size_t,2> > onbbox(onewmin,onewmax);
                
                return std::pair<Image,Image>(subImage(nbbox),other.subImage(onbbox));
            }
            
        };
    }
}

#endif
