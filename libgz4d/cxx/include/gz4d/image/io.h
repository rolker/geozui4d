#ifndef GZ4D_IMAGE_IO_H
#define GZ4D_IMAGE_IO_H

#include <gz4d/Image.h>
#include <gz4d/image/PNG.h>
#include <gz4d/image/TIFF.h>
#include "JPEG.h"
#include <boost/algorithm/string.hpp>

namespace gz4d
{
    namespace image
    {
        inline std::string filetype(std::string const &fname)
        {
            std::string e = boost::algorithm::to_lower_copy(boost::filesystem::extension(fname));
            if(e == ".png")
                return "png";
            if (e == ".tif" || e == ".tiff")
                return "tiff";
            if (e == ".jpg" || e == ".jpeg")
                return "jpeg";
            throw Exception("Unknown image file type");
        }

        inline Info info(std::string const &fname)
        {
            std::string t = filetype(fname);
            if(t == "png")
                return PNG(fname).GetInfo();
            if(t == "tiff")
                return Tiff(fname).GetInfo();
            if(t == "jpeg")
                return JPEG(fname).GetInfo();
            return Info();
        }


        template <typename T> inline T load(T &i, std::string const &fname)
        {
            std::string t = filetype(fname);
            if(t == "png")
                i = PNG(fname).Load<T>();
            else if(t == "tiff")
                i = Tiff(fname).Load<T>();
            else if(t == "jpeg")
                i = JPEG(fname).Load<T>();
            return i;
        }

        template <typename T> inline void save(T const &i, std::string const &fname)
        {
            std::string t = filetype(fname);
            if(t == "png")
                PNG(fname).Save(i);
            if(t == "tiff")
                Tiff(fname).Save(i);
            if(t == "jpeg")
                JPEG(fname).Save(i);
        }
    }
}

template <typename T> void gz4d::TypedImage<T>::Load(std::string const &fn)
{
    image::load<TypedImage<T> >(*this,fn);
}

template <typename T> void gz4d::TypedImage<T>::Save(std::string const &fn)
{ 
    image::save(*this,fn);
}

#endif
