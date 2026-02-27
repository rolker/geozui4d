#ifndef GZ4D_EXIF_H
#define GZ4D_EXIF_H

//gz4d_cmake depends exif

#include <gz4d/gz4d.h>
#include <memory>
#include <gz4d/Vector.h>
#include <libexif/exif-data.h>
#include <gz4d/util/Utilities.h>


namespace gz4d
{
    class Exif
    {
        std::shared_ptr<_ExifData> data;

        _ExifContent *GetContent(std::string const &ifd)
        {
            ExifContent *c = NULL;
            if(ifd == "0")
                c = data->ifd[EXIF_IFD_0];
            if(!c && ifd == "1")
                c = data->ifd[EXIF_IFD_1];
            if(!c && ifd == "exif")
                c = data->ifd[EXIF_IFD_EXIF];
            if(!c && ifd == "gps")
                c = data->ifd[EXIF_IFD_GPS];
            if(!c && ifd == "interoperability")
                c = data->ifd[EXIF_IFD_INTEROPERABILITY];
            if(!c && ifd == "count")
                c = data->ifd[EXIF_IFD_COUNT];
            return c;
        }

        _ExifEntry *GetEntry(std::string const &ifd, std::string const &entry)
        {
            ExifContent *c = GetContent(ifd);
            if(!c)
                return NULL;

            return exif_content_get_entry(c,exif_tag_from_name(entry.c_str()));
        }


        public:
            Exif(){}
            Exif(Exif const &e):data(e.data){}
            Exif(std::string const &f):data(exif_data_new_from_file(f.c_str()), exif_data_free){}

            std::string Get(std::string const &ifd, std::string const &entry)
            {
                ExifEntry* e = GetEntry(ifd,entry);
                if(!e)
                    return "";

                char val[256];
                exif_entry_get_value(e,val,256);
                return val;
            }


            std::string GetString(std::string const &ifd, std::string const &entry)
            {
                ExifEntry* e = GetEntry(ifd,entry);
                
                if(!e || e->format != EXIF_FORMAT_ASCII)
                    return "";
                return reinterpret_cast<const char*>(e->data);
            }
            
            float GetFloat(std::string const &ifd, std::string const &entry)
            {
                ExifEntry* e = GetEntry(ifd,entry);
                if(!e)
                    return Nan<float>();
                
                if(e->format == EXIF_FORMAT_RATIONAL)
                {
                    ExifRational r = exif_get_rational(e->data, exif_data_get_byte_order(data.get()));
                    return r.numerator/float(r.denominator);
                }
                
                if(e->format == EXIF_FORMAT_SRATIONAL)
                {
                    ExifSRational r = exif_get_srational(e->data, exif_data_get_byte_order(data.get()));
                    return r.numerator/float(r.denominator);
                }
            }
            
            uint16_t GetShort(std::string const &ifd, std::string const &entry)
            {
                ExifEntry* e = GetEntry(ifd,entry);
                
                if(!e || e->format != EXIF_FORMAT_SHORT)
                    return 0;
                return exif_get_short(e->data, exif_data_get_byte_order(data.get()));
            }
            

            Vector<float,2> GetFOV()
            {
                float f = GetFloat("exif","FocalLength");
                
                uint16_t x = GetShort("exif","PixelXDimension");
                uint16_t y = GetShort("exif","PixelYDimension");
                
                float xri = GetFloat("exif","FocalPlaneXResolution");
                float yri = GetFloat("exif","FocalPlaneYResolution");
                
                float xrm = xri / 0.0254;
                float yrm = yri / 0.0254;
                
                float ipx = x/xrm;
                float ipy = y/yrm;
                
                return Vector<float,2>(Degrees(2.0*atan((ipx/2.0)/(f/1000.0))),Degrees(2.0*atan((ipy/2.0)/(f/1000.0))));
            }
    };
}

#endif
