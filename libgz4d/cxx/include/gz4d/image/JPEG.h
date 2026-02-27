#ifndef GZ4D_IMAGE_JPEG_H
#define GZ4D_IMAGE_JPEG_H

//gz4d_cmake depends JPEG

#include <gz4d/gz4d.h>
#include <gz4d/image/common.h>
#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>
#include <gz4d/image/Info.h>

#include <filesystem>
#include <memory>

#ifdef BOOST_MSVC
#include <basetsd.h>
#endif

#include <jpeglib.h>
#include <cstdio>


namespace gz4d
{
    namespace image
    {

        struct JPEG_ReadContext
        {
            jpeg_decompress_struct jpeg;
            jpeg_error_mgr jerr;
            std::shared_ptr<FILE> file;
            bool decompress;
            JPEG_ReadContext(std::string const &fname, bool decomp)
            :decompress(decomp)
            {
                if (!exists(std::filesystem::path(fname)))
                    throw Exception("File not found");
                
                file = std::shared_ptr<FILE>(fopen(fname.c_str(),"rb"),fclose);
                if(!file)
                    throw(Exception(std::string("Error opening file: ")+fname));
                jpeg.err = jpeg_std_error(&jerr);
                jpeg_create_decompress(&jpeg);
                jpeg_stdio_src(&jpeg, file.get());
                jpeg_read_header(&jpeg, true);
                jpeg_calc_output_dimensions(&jpeg);
                if(decompress)
                    jpeg_start_decompress(&jpeg);
                
            }
            ~JPEG_ReadContext()
            {
                if(decompress)
                    jpeg_finish_decompress(&jpeg);
                jpeg_destroy_decompress(&jpeg);
            }
            
        };
        
        struct JPEG_WriteContext
        {
            jpeg_compress_struct jpeg;
            jpeg_error_mgr jerr;
            std::shared_ptr<FILE> file;
            JPEG_WriteContext(std::string const &fname, std::size_t width, std::size_t height)
            {
                
                file = std::shared_ptr<FILE>(fopen(fname.c_str(),"wb"),fclose);
                if(!file)
                    throw(Exception(std::string("Error opening file: ")+fname));
                jpeg.err = jpeg_std_error(&jerr);
                jpeg_create_compress(&jpeg);
                jpeg_stdio_dest(&jpeg, file.get());
                jpeg.image_width = width;
                jpeg.image_height = height;
            }
            ~JPEG_WriteContext()
            {
                jpeg_destroy_compress(&jpeg);
            }
            
        };
        
        
        class JPEG
        {
            std::string fname;
            Raster<Pixel<L8> >::Ptr LoadL8()
            {
                JPEG_ReadContext jc(fname,true);
                
                std::size_t width = jc.jpeg.output_width;
                std::size_t height = jc.jpeg.output_height;
                if(jc.jpeg.out_color_space != JCS_GRAYSCALE)
                    throw(Exception(std::string("Not a grayscale image: ")+fname));
                
                Raster<Pixel<L8> >::Ptr r(new Raster<Pixel<L8> >);
                r->Resize(width,height);
                for(std::size_t i = 0; i < height; ++i)
                {
                    JSAMPLE * row = reinterpret_cast<JSAMPLE*>(r->GetRow(height-1-i));
                    jpeg_read_scanlines(&jc.jpeg, &row, 1);
                }
                return r;
            }
            
            Raster<Pixel<RGB8> >::Ptr LoadRGB8()
            {
                JPEG_ReadContext jc(fname,true);
                
                std::size_t width = jc.jpeg.output_width;
                std::size_t height = jc.jpeg.output_height;
                
                Raster<Pixel<RGB8> >::Ptr r(new Raster<Pixel<RGB8> >);
                r->Resize(width,height);
                for(std::size_t i = 0; i < height; ++i)
                {
                    JSAMPLE * row = reinterpret_cast<JSAMPLE*>(r->GetRow(height-1-i));
                    jpeg_read_scanlines(&jc.jpeg, &row, 1);
                }
                return r;
            }
            
            void SaveL8(Raster<Pixel<L8> > &r)
            {
                std::size_t height = r.GetRows();
                JPEG_WriteContext jc(fname,r.GetColumns(),r.GetRows());
                
                jc.jpeg.input_components = 1;
                jc.jpeg.in_color_space = JCS_GRAYSCALE;
                jpeg_set_defaults(&jc.jpeg);
                jpeg_start_compress(&jc.jpeg, true);
                for(std::size_t row = 0; row < height; ++row)
                {
                    JSAMPLE * rowptr = reinterpret_cast<JSAMPLE*>(r.GetRow(height-1-row));
                    jpeg_write_scanlines(&jc.jpeg, &rowptr, 1);
                }
                jpeg_finish_compress(&jc.jpeg);
                
            }
            
            void SaveRGB8(Raster<Pixel<RGB8> > &r)
            {
                std::size_t height = r.GetRows();
                JPEG_WriteContext jc(fname,r.GetColumns(),height);
                
                jc.jpeg.input_components = 3;
                jc.jpeg.in_color_space = JCS_RGB;
                jpeg_set_defaults(&jc.jpeg);
                jpeg_start_compress(&jc.jpeg, true);
                for(std::size_t row = 0; row < height; ++row)
                {
                    JSAMPLE * rowptr = reinterpret_cast<JSAMPLE*>(r.GetRow(height-1-row));
                    jpeg_write_scanlines(&jc.jpeg, &rowptr, 1);
                }
                jpeg_finish_compress(&jc.jpeg);
            }

            public:
                JPEG(std::string const &fname):fname(fname){}
                Info GetInfo()
                {
                    JPEG_ReadContext jc(fname,false);
                    
                    Info ret;
                    ret.width = jc.jpeg.output_width;
                    ret.height = jc.jpeg.output_height;
                    switch(jc.jpeg.out_color_space)
                    {
                        case JCS_GRAYSCALE:
                            ret.format = "L";
                            if(jc.jpeg.out_color_components != 1)
                                ret.format += "(? reports " + boost::lexical_cast<std::string>(jc.jpeg.out_color_components) + " channels)";
                            break;
                        case JCS_RGB:
                            ret.format = "RGB";
                            if(jc.jpeg.out_color_components != 3)
                                ret.format += "(? reports " + boost::lexical_cast<std::string>(jc.jpeg.out_color_components) + " channels)";
                            break;
                        case JCS_YCbCr:
                            ret.format = "YUV";
                            if(jc.jpeg.out_color_components != 3)
                                ret.format += "(? reports " + boost::lexical_cast<std::string>(jc.jpeg.out_color_components) + " channels)";
                            break;
                        case JCS_CMYK:
                            ret.format = "CMYK";
                            if(jc.jpeg.out_color_components != 4)
                                ret.format += "(? reports " + boost::lexical_cast<std::string>(jc.jpeg.out_color_components) + " channels)";
                            break;
                        case JCS_YCCK:
                            ret.format = "YCCK";
                            if(jc.jpeg.out_color_components != 4)
                                ret.format += "(? reports " + boost::lexical_cast<std::string>(jc.jpeg.out_color_components) + " channels)";
                            break;
                        default:
                            ret.format = "unknown";
                    }
                    ret.format += boost::lexical_cast<std::string>(BITS_IN_JSAMPLE);
                    return ret;
                }

                template <typename T> T Load();
                template <typename T> void Save(T const &i);
        };


        template <typename T> inline T JPEG::Load()
        {
            Info i = GetInfo();
            if(i.format.find("L") == 0)
            {
                Raster<Pixel<L8> >::Ptr r = LoadL8();
                return T(*r);
            }
            if(i.format.find("RGB") == 0) // starts with RGB
            {
                Raster<Pixel<RGB8> >::Ptr r = LoadRGB8();
                return T(*r);
            }
            throw(Exception("Unsupported Jpeg format"));
        }

        template <typename T> inline void JPEG::Save(T const &i)
        {
            if(typename T::pixel_format().HasLuminance())
            {
                Raster<Pixel<L8> > r(i.GetRaster());
                SaveL8(r);
            }
            else
            {
                Raster<Pixel<RGB8> > r (i.GetRaster());
                SaveRGB8(r);
            }

        }
    }
}

#endif
