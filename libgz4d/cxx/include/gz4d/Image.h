// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_IMAGE_H
#define GZ4D_IMAGE_H

#include <gz4d/gz4d.h>
#include <gz4d/data/Raster.h>
#include <gz4d/image/Info.h>
#include <gz4d/Pixel.h>
#include <gz4d/data/binary/Packet.h>
#include <gz4d/vector/operations.h>

namespace gz4d
{

    template <typename T> class TypedImage
    {
        protected:
            typename Raster<T>::Ptr raster;
        public:
            typedef T pixel_type;
            typedef typename T::pixel_format pixel_format;
            typedef Raster<T> raster_type;
            static const std::size_t raw_pixel_size = pixel_format::raw_size; 

            TypedImage():raster(){}
            TypedImage(TypedImage const &other) :raster(other.raster){}
            TypedImage(typename Raster<T>::Ptr &r):raster(r){}
            TypedImage(Vector<std::size_t,2> const &s, T const &init):raster(new Raster<T>(s,init)){}
            TypedImage(std::size_t c, std::size_t r, T const &init):raster(new Raster<T>(c,r,init)){}
            TypedImage(std::size_t c, std::size_t r, TypedImage const &o):raster(new Raster<T>)
            {
                raster->Resize(c,r);
                double xscale = o.raster->GetColumns()/double(c);
                double yscale = o.raster->GetRows()/double(r);
                for(std::size_t y = 0; y < r; ++y)
                {
                    std::size_t sy0 = floor(y*yscale);
                    std::size_t sy1 = ceil(y*yscale);
                    double sy0p = sy1-y*yscale;
                    double sy1p = 1.0 - sy0p;
                    for(std::size_t x = 0; x < c; ++x)
                    {
                        std::size_t sx0 = floor(x*xscale);
                        std::size_t sx1 = ceil(x*xscale);
                        double sx0p = sx1-x*xscale;
                        double sx1p = 1.0 - sx0p;
                        raster->GetRow(y)[x] = Color(o.raster->GetRow(sy0)[sx0])*sy0p*sx0p+
                                              Color(o.raster->GetRow(sy1)[sx0])*sy1p*sx0p+
                                              Color(o.raster->GetRow(sy0)[sx1])*sy0p*sx1p+
                                              Color(o.raster->GetRow(sy1)[sx1])*sy1p*sx1p;
                    }
                }
            }

            template <typename OT> TypedImage(Raster<OT> const &other) :raster(new Raster<T>(other)){}

            void Resize(std::size_t width, std::size_t height) {if(!raster) raster = typename Raster<T>::Ptr(new Raster<T>); raster->Resize(width,height);}
            void Resize(Vector<std::size_t,2> const &s){Resize(s[0],s[1]);}

            image::Info GetInfo() const {return image::Info(raster->GetColumns(),raster->GetRows(),pixel_format().String());}

            std::size_t width() const {return raster->GetColumns();}
            std::size_t height() const {return raster->GetRows();}
            Vector<std::size_t,2> getSizes() const {return raster->getSizes();}

            typename pixel_format::raw_type* raw() const {return reinterpret_cast<typename pixel_format::raw_type*>(raster->raw());}
            //static std::size_t raw_pixel_size() {return pixel_format::raw_size;} 
            std::size_t raw_size() const {return raster->raw_size();}
            typename Raster<T>::Ptr &GetRasterPtr() {return raster;}
            Raster<T> const &GetRaster() const {return *raster;}
            Raster<T> &GetRaster() {return *raster;}
            operator Raster<T> const &() const {return *raster;}
            operator bool() const {return bool(raster);}
            bool empty() const {return !raster || raster->empty();}
            void reset() {raster.reset();}

            inline void Load(std::string const &fn);
            inline void Save(std::string const &fn);// { image::save(*this,fn);}
            
            Color getInterpolated(const Vector<double,2> &index, const Color &nullColor) const
            {
                if(index[0] < 0 || index[0] >= width() || index[1] < 0 || index[1] >= height())
                    return Color(0,0,0,0);
                Vector<int,2> i0 = floor(index);
                Color c00=nullColor;
                Color c01=nullColor;
                Color c10=nullColor;
                Color c11=nullColor;
                if (i0[0] >= 0 && i0[0] < width())
                {
                    if(i0[1] >= 0 && i0[1] < height())
                        c00 = raster->GetRow(i0[1])[i0[0]];
                    else
                        return nullColor;
                    if(i0[1] >= -1 && i0[1] < height()-1)
                        c01 = raster->GetRow(i0[1]+1)[i0[0]];
                    else
                        return nullColor;
                }
                if (i0[0] >= -1 && i0[0] < width()-1)
                {
                    if(i0[1] >= 0 && i0[1] < height())
                        c10 = raster->GetRow(i0[1])[i0[0]+1];
                    else
                        return nullColor;
                    
                    if(i0[1] >= -1 && i0[1] < height()-1)
                        c11 = raster->GetRow(i0[1]+1)[i0[0]+1];
                    else
                        return nullColor;
                }
                Vector< double, 2 > p = index-i0;
                return c00.Blend(c10,p[0]).Blend(c01.Blend(c11,p[0]),p[1]);
            }
    };

    typedef TypedImage<Pixel<RGB8> > ImageRGB8;
    typedef TypedImage<Pixel<BGR8> > ImageBGR8;
    typedef TypedImage<Pixel<RGBA8> > ImageRGBA8;
    typedef TypedImage<Pixel<RGB16> > ImageRGB16;
    typedef TypedImage<Pixel<RGBA16> > ImageRGBA16;
    typedef TypedImage<Pixel<L8> > ImageL8;
    typedef TypedImage<Pixel<LA8> > ImageLA8;
    typedef TypedImage<Pixel<L16> > ImageL16;
    typedef TypedImage<Pixel<LA16> > ImageLA16;
    typedef TypedImage<Pixel<L32> > ImageL32;
    
    typedef TypedImage<Pixel<RGBA5_5_5_1> > ImageRGBA5_5_5_1;
    typedef TypedImage<Pixel<YUV8> > ImageYUV8;

    static uint32_t fourcc(std::string const &f)
    {
        return *reinterpret_cast<uint32_t const*>(f.c_str());
    }

    static std::string fourcc(uint32_t f)
    {
        char ret[5];
        *reinterpret_cast<uint32_t*>(ret) = f;
        ret[4] = 0;
        return ret;
    }

    static ImageYUV8 toImage(binary::Image::Frame const &frame)
    {
        ImageYUV8 ret;
        if(frame.fourcc.value == fourcc("YUYV"))
        {
            ret.Resize(frame.width,frame.height);
            uint32_t bytesPerLine = frame.dataSize/frame.height;
            for(int r = 0; r < frame.height; ++r)
                for(int c = 0; c < frame.width; c+=2)
                {
                    uint8_t const * yuv = &frame.data[r*bytesPerLine+c*2];
                    Pixel<YUV8> p;
                    p[1] = yuv[1];
                    p[2] = yuv[3];
                    for(int i = 0; i < 2; ++i)
                    {
                        p[0] = yuv[i*2];
                        ret.GetRaster().GetRow(frame.height-r-1)[c+i] = p;
                    }
                }
        }
        return ret;
    }
};

#endif
