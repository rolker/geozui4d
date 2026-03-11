// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_IMAGE_PNG_H
#define GZ4D_IMAGE_PNG_H

//gz4d_cmake depends PNG

#include <gz4d/gz4d.h>
#include <gz4d/image/common.h>

#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>
#include <gz4d/image/Info.h>

#include <filesystem>
#include <memory>

#include <png.h>


namespace gz4d
{
    namespace image
    {

        struct PNG_Handle
        {
            png_structp png;
            png_infop info;
            png_infop end;
            std::shared_ptr<FILE> file;
            png_uint_32 width;
            png_uint_32 height;
            int bit_depth;
            int color_type;
            
        };
        
        struct PNG_ReadHandle: public PNG_Handle
        {
            
            PNG_ReadHandle(std::string const &fname)
            {
                if (!exists(std::filesystem::path(fname)))
                    throw Exception("File not found");
                
                file = std::shared_ptr<FILE>(fopen(fname.c_str(),"rb"),fclose);
                if(!file)
                    throw(Exception(std::string("Error opening file: ")+fname));
                
                png_byte head[8];
                if(fread(head,1,8,file.get()) != 8)
                {
                    file.reset();
                    throw(Exception(std::string("Error reading header: ")+fname));
                }
                if(png_sig_cmp(head, 0, 8))
                {
                    file.reset();
                    throw(Exception(std::string("Doesn't seem to be a PNG file: ")+fname));
                }
                
                png = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
                info = png_create_info_struct(png);
                end = png_create_info_struct(png);
                setjmp(png_jmpbuf(png));
                
                png_init_io(png,file.get());
                png_set_sig_bytes(png,8);
                png_read_info(png,info);
                png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
                
                if (color_type == PNG_COLOR_TYPE_PALETTE)
                {
                    png_set_palette_to_rgb(png);
                    color_type = PNG_COLOR_TYPE_RGB;
                }
                
                if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
                {
                    png_set_expand_gray_1_2_4_to_8(png);
                    bit_depth = 8;
                }
                
                if (png_get_valid(png, info,PNG_INFO_tRNS))
                    png_set_tRNS_to_alpha(png);
                
                if (bit_depth < 8)
                {
                    png_set_packing(png);
                    bit_depth = 8;
                }
                
                if (bit_depth == 16)
                    png_set_swap(png);
                
                png_set_interlace_handling(png);
                
                png_read_update_info(png, info);
            }
            
            ~PNG_ReadHandle()
            {
                png_destroy_read_struct(&png, &info, &end);
            }
            
            template <typename FT> typename Raster<Pixel<FT> >::Ptr Load()
            {
                typename Raster<Pixel<FT> >::Ptr r(new Raster<Pixel<FT> >);
                std::vector<png_byte*> rows(height);
                
                r->Resize(width,height);
                for(std::size_t i = 0; i < height; ++i)
                    rows[i] = reinterpret_cast<png_byte*>(r->GetRow(height-1-i));
                png_read_image(png,&rows.front());
                return r;
            }
        };
        
        class PNG_WriteHandle: public PNG_Handle
        {
            public:
                PNG_WriteHandle(std::string const &fname)
                {
                    file = std::shared_ptr<FILE>(fopen(fname.c_str(),"wb"),fclose);
                    if(!file)
                        throw(Exception(std::string("Error opening file: ")+fname));
                    
                    png = png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
                    info = png_create_info_struct(png);
                    end = NULL;
                    setjmp(png_jmpbuf(png));
                    
                    png_init_io(png,file.get());
                    bit_depth = 0;
                    color_type = PNG_COLOR_TYPE_GRAY;
                    
                }
                
                ~PNG_WriteHandle()
                {
                    png_destroy_write_struct(&png, &info);
                }
                
                void Save(png_byte * data, int pixel_size)
                {
                    png_set_IHDR(png, info, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
                    
                    std::vector<png_byte *> rows;
                    
                    for(std::size_t i = height-1; i > 0; --i)
                        rows.push_back(&data[i*pixel_size*width]);
                    rows.push_back(data);
                    
                    png_set_rows(png, info,&rows.front());
                    
                    int transforms = PNG_TRANSFORM_IDENTITY;
                    if(bit_depth == 16)
                        transforms |= PNG_TRANSFORM_SWAP_ENDIAN;
                    
                    png_write_png(png, info,transforms,NULL);
                    png_write_end(png, info);
                }
                
        };
        
        class PNG
        {

            std::string fname;

            inline Raster<Pixel<LA8> >::Ptr LoadLA8();
            inline Raster<Pixel<L8> >::Ptr LoadL8();
            inline Raster<Pixel<L16> >::Ptr LoadL16();
            inline Raster<Pixel<RGBA8> >::Ptr LoadRGBA8();
            inline Raster<Pixel<RGB8> >::Ptr LoadRGB8();
            inline void SaveL8(Raster<Pixel<L8> > &r);
            inline void SaveRGB8(Raster<Pixel<RGB8> > &r);
            inline void SaveLA8(Raster<Pixel<LA8> > &r);
            inline void SaveRGBA8(Raster<Pixel<RGBA8> > &r);
            public:
                PNG(std::string const &fname):fname(fname){}
                inline Info GetInfo();
                template <typename T> inline T Load();
                template <typename T> inline void Save(T const &i);
        };


        template <typename T> inline T PNG::Load()
        {
            Info i = GetInfo();
            if(i.format == "LA8")
            {
                Raster<Pixel<LA8> >::Ptr r = LoadLA8();
                return T(*r);
            }
            if(i.format == "L8")
            {
                Raster<Pixel<L8> >::Ptr r = LoadL8();
                return T(*r);
            }
            if(i.format == "L16")
            {
                Raster<Pixel<L16> >::Ptr r = LoadL16();
                return T(*r);
            }
            if(i.format == "RGBA8")
            {
                Raster<Pixel<RGBA8> >::Ptr r = LoadRGBA8();
                return T(*r);
            }
            if(i.format == "RGB8")
            {
                Raster<Pixel<RGB8> >::Ptr r = LoadRGB8();
                return T(*r);
            }
            return T();
        }

        template <typename T> inline void PNG::Save(T const &i)
        {
            if(typename T::pixel_format().HasLuminance())
            {
                if(typename T::pixel_format().HasAlpha())
                {
                    Raster<Pixel<LA8> > r(i.GetRaster());
                    SaveLA8(r);
                }
                else
                {
                    Raster<Pixel<L8> > r(i.GetRaster());
                    SaveL8(r);
                }
            }
            else
            {
                if(typename T::pixel_format().HasAlpha())
                {
                    Raster<Pixel<RGBA8> > r (i.GetRaster());
                    SaveRGBA8(r);
                }
                else
                {
                    Raster<Pixel<RGB8> > r (i.GetRaster());
                    SaveRGB8(r);
                }
            }

        }


    }
}

void gz4d::image::PNG::SaveL8(Raster<Pixel<L8> > &r)
{
    PNG_WriteHandle png(fname);
    png.width = r.GetColumns();
    png.height = r.GetRows();
    png.bit_depth = 8;
    png.color_type = PNG_COLOR_TYPE_GRAY;
    png.Save(reinterpret_cast<png_byte *>(r.raw()),1);
}

void gz4d::image::PNG::SaveRGB8(Raster<Pixel<RGB8> > &r)
{
    PNG_WriteHandle png(fname);
    png.width = r.GetColumns();
    png.height = r.GetRows();
    png.bit_depth = 8;
    png.color_type = PNG_COLOR_TYPE_RGB;
    png.Save(reinterpret_cast<png_byte *>(r.raw()),3);
}

void gz4d::image::PNG::SaveLA8(Raster<Pixel<LA8> > &r)
{
    PNG_WriteHandle png(fname);
    png.width = r.GetColumns();
    png.height = r.GetRows();
    png.bit_depth = 8;
    png.color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
    png.Save(reinterpret_cast<png_byte *>(r.raw()),2);
}

void gz4d::image::PNG::SaveRGBA8(Raster<Pixel<RGBA8> > &r)
{
    PNG_WriteHandle png(fname);
    png.width = r.GetColumns();
    png.height = r.GetRows();
    png.bit_depth = 8;
    png.color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    png.Save(reinterpret_cast<png_byte *>(r.raw()),4);
}

gz4d::image::Info gz4d::image::PNG::GetInfo()
{
    PNG_ReadHandle png(fname);
    Info ret;
    switch (png.color_type)
    {
        case PNG_COLOR_TYPE_GRAY:
            ret.format = "L";
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            ret.format = "LA";
            break;
        case PNG_COLOR_TYPE_RGB:
            ret.format = "RGB";
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            ret.format = "RGBA";
            break;
    }
    if (png.bit_depth == 16)
        ret.format += "16";
    else
        ret.format += "8";
    ret.width = png.width;
    ret.height = png.height;
    return ret;
}


gz4d::Raster<gz4d::Pixel<gz4d::L8> >::Ptr gz4d::image::PNG::LoadL8()
{
    return PNG_ReadHandle(fname).Load<L8>();
}

gz4d::Raster<gz4d::Pixel<gz4d::LA8> >::Ptr gz4d::image::PNG::LoadLA8()
{
    return PNG_ReadHandle(fname).Load<LA8>();
}

gz4d::Raster<gz4d::Pixel<gz4d::L16> >::Ptr gz4d::image::PNG::LoadL16()
{
    return PNG_ReadHandle(fname).Load<L16>();
}


gz4d::Raster<gz4d::Pixel<gz4d::RGB8> >::Ptr gz4d::image::PNG::LoadRGB8()
{
    return PNG_ReadHandle(fname).Load<RGB8>();
}

gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> >::Ptr gz4d::image::PNG::LoadRGBA8()
{
    return PNG_ReadHandle(fname).Load<RGBA8>();
}



#endif
