#ifndef GZ4D_IMAGE_TIFF_H
#define GZ4D_IMAGE_TIFF_H

//gz4d_cmake depends TIFF

#include <gz4d/gz4d.h>
#include <gz4d/image/common.h>

#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>
#include <gz4d/image/Info.h>

#include <filesystem>
#include <memory>

#include <tiffio.h>

namespace gz4d
{
    namespace image
    {

        struct TIFFMemoryIO
        {
            std::vector<uint8_t> _data;
            std::vector<uint8_t>::iterator _cursor;
            
            TIFFMemoryIO(std::vector<uint8_t> &d):_data(d),_cursor(_data.begin())
            {
            }
            
            TIFFMemoryIO():_cursor(_data.begin())
            {
            }
            
            static tsize_t TIFFRead(thandle_t s, tdata_t data, tsize_t size)
            {
                TIFFMemoryIO *self = reinterpret_cast<TIFFMemoryIO*>(s);
                uint8_t *out_start = reinterpret_cast<uint8_t *>(data);
                uint8_t *out_cursor = out_start;
                while(self->_cursor <= self->_data.end() && out_cursor-out_start < size)
                {
                    *out_cursor = *(self->_cursor);
                    ++out_cursor;
                    ++(self->_cursor);
                }
                return out_cursor-out_start;
            }
            
            static tsize_t TIFFWrite(thandle_t s, tdata_t data, tsize_t size)
            {
                TIFFMemoryIO *self = reinterpret_cast<TIFFMemoryIO*>(s);
                if(size > self->_data.end()-self->_cursor)
                {
                    toff_t c = self->_cursor-self->_data.begin();
                    self->_data.resize(c+size);
                    self->_cursor = self->_data.begin()+c;
                }
                memcpy(&(*(self->_cursor)),data,size);
                return size;
            }
            
            static toff_t TIFFSeek(thandle_t s, toff_t offset, int whence)
            {
                TIFFMemoryIO *self = reinterpret_cast<TIFFMemoryIO*>(s);
                switch(whence)
                {
                    case SEEK_SET:
                        self->_cursor = self->_data.begin()+offset;
                        break;
                    case SEEK_CUR:
                        self->_cursor += offset;
                        break;
                    case SEEK_END:
                        self->_cursor = self->_data.end()+offset;
                        break;
                }
                return self->_cursor-self->_data.begin();
            }
            
            static int TIFFClose(thandle_t s)
            {
                return 0;
            }
            
            static toff_t TIFFSize(thandle_t s)
            {
                TIFFMemoryIO *self = reinterpret_cast<TIFFMemoryIO*>(s);
                return self->_data.size();
            }
            
        };
        
        struct TIFF_WriteContext
        {
            TIFF *tiff;
            typedef std::shared_ptr<TIFF_WriteContext> Ptr;
            private:
                void init(TIFF * t,std::string const& format, uint32_t width, uint32_t height)
                {
                    tiff = t;
            
                    uint16_t bit_depth = 0;
                    
                    if(RGB8()(format) || RGBA8()(format) || L8()(format) || LA8()(format))
                        bit_depth = 8;
                    if(RGB16()(format) || RGBA16()(format) || L16()(format) || LA16()(format))
                        bit_depth = 16;
                    
                    
                    if(L8()(format) || L16()(format))
                    {
                        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
                        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1); 
                    }
                    if(LA8()(format) || LA16()(format))
                    {
                        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
                        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 2); 
                        TIFFSetField(tiff, TIFFTAG_EXTRASAMPLES, EXTRASAMPLE_UNASSALPHA);
                    }
                    if(RGB8()(format) || RGB16()(format))
                    {
                        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
                        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 3); 
                    }
                    if(RGBA8()(format) || RGBA16()(format))
                    {
                        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
                        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4); 
                        uint16 es = EXTRASAMPLE_UNASSALPHA;
                        TIFFSetField(tiff, TIFFTAG_EXTRASAMPLES, 1, &es);
                    }
                    
                    TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width);
                    TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height);
                    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, bit_depth);
                    TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
                    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
                }
            public:
            
                TIFF_WriteContext(std::string const &fname, std::string const& format, uint32_t width, uint32_t height)
                {
                    TIFF* t = TIFFOpen(fname.c_str(),"w");
                    if(!t)
                        throw(Exception(std::string("Error opening file: ")+fname));
                    init(t,format,width,height);
                }

                TIFF_WriteContext(TIFFMemoryIO &mem, std::string const& format, uint32_t width, uint32_t height)
                {
                    TIFF* t = TIFFClientOpen("","w",&mem,&TIFFMemoryIO::TIFFRead,&TIFFMemoryIO::TIFFWrite,&TIFFMemoryIO::TIFFSeek,&TIFFMemoryIO::TIFFClose,&TIFFMemoryIO::TIFFSize,NULL,NULL);
                    if(!t)
                        throw(Exception("Error opening memory"));
                    init(t,format,width,height);
                }
                
                ~TIFF_WriteContext()
                {
                    TIFFClose(tiff);
                }
        };

        struct TIFF_ReadContext
        {
            std::shared_ptr<TIFF> tiff;
            uint32_t w,h;
            private:
                void init(TIFF * t)
                {
                    tiff = std::shared_ptr<TIFF>(t,TIFFClose);
                    TIFFGetField(tiff.get(), TIFFTAG_IMAGEWIDTH, &w);
                    TIFFGetField(tiff.get(), TIFFTAG_IMAGELENGTH, &h);
                }
            
            public:
                TIFF_ReadContext(std::string const &fname)
                {
                    if (!exists(std::filesystem::path(fname)))
                        throw Exception("File not found");
                
                    TIFF* tiff = TIFFOpen(fname.c_str(),"r");
                    if(!tiff)
                        throw(Exception(std::string("Error opening file: ")+fname));
                    init(tiff);
                }

                TIFF_ReadContext(TIFFMemoryIO &mem)
                {
                    TIFF* t = TIFFClientOpen("","r",&mem,&TIFFMemoryIO::TIFFRead,&TIFFMemoryIO::TIFFWrite,&TIFFMemoryIO::TIFFSeek,&TIFFMemoryIO::TIFFClose,&TIFFMemoryIO::TIFFSize,NULL,NULL);
                    if(!t)
                        throw(Exception("Error opening memory"));
                    init(t);
                }

                Info GetInfo()
                {
                    Info ret;
                    ret.width = w;
                    ret.height = h;
                    
                    uint16_t spp;
                    if(TIFFGetFieldDefaulted(tiff.get(),TIFFTAG_SAMPLESPERPIXEL, &spp))
                    {
                        switch(spp)
                        {
                            case 1:
                                ret.format = "L";
                                break;
                            case 2:
                                ret.format = "LA";
                                break;
                            case 3:
                                ret.format = "RGB";
                                break;
                            case 4:
                                ret.format = "RGBA";
                                break;
                            default:
                                ret.format = "Unknown";
                        }
                    }
                    else
                        ret.format = "Unknown";
                    
                    uint16_t bpp;
                    if(TIFFGetFieldDefaulted(tiff.get(), TIFFTAG_BITSPERSAMPLE, &bpp))
                    {
                        ret.format += boost::lexical_cast<std::string>(bpp);
                    }
                    else
                        ret.format += "?";
                    
                    return ret;
                }
                
                Raster<Pixel<RGBA8> >::Ptr LoadRGBA8()
                {
                    Raster<Pixel<RGBA8> >::Ptr r(new Raster<Pixel<RGBA8> >);
                    r->Resize(w,h);
                    if(!TIFFReadRGBAImageOriented(tiff.get(),w,h,reinterpret_cast<uint32*>(r->raw()),ORIENTATION_BOTLEFT,0))
                        throw(Exception("Error reading image data"));
                    return r;
                }
                
                
        };
                
        
        class Tiff
        {
            std::string fname;
            TIFFMemoryIO * memio;

            template <typename PF> void SaveRaster(Raster<Pixel<PF> > &r)
            {
                TIFF_WriteContext::Ptr wc;
                if(memio)
                    wc = TIFF_WriteContext::Ptr(new TIFF_WriteContext(*memio,PF::String(),r.GetColumns(),r.GetRows()));
                else
                    wc = TIFF_WriteContext::Ptr(new TIFF_WriteContext(fname,PF::String(),r.GetColumns(),r.GetRows()));
                for(std::size_t row = 0; row < r.GetRows(); ++row)
                    TIFFWriteScanline(wc->tiff, const_cast<void *>(reinterpret_cast<void const *>(r.GetRow(r.GetRows()-1-row))),row);
            }            
            
            public:
                Tiff(std::string const &fname):fname(fname),memio(NULL){}
                Tiff(TIFFMemoryIO &m):memio(&m){}
                Info GetInfo()
                {
                    if (memio)
                        return TIFF_ReadContext(*memio).GetInfo();
                    return TIFF_ReadContext(fname).GetInfo();
                }
                
                template <typename T> T Load()
                {
                    Raster<Pixel<RGBA8> >::Ptr r;
                    if (memio)
                        r=TIFF_ReadContext(*memio).LoadRGBA8();
                    else
                        r=TIFF_ReadContext(fname).LoadRGBA8();
                    return T(*r);
                }
                
                template <typename T> void Save(T const &i)
                {
                    if(typename T::pixel_format().HasLuminance())
                    {
                        if(typename T::pixel_format().HasAlpha())
                        {
                            Raster<Pixel<LA8> > r(i.GetRaster());
                            SaveRaster(r);
                        }
                        else
                        {
                            Raster<Pixel<L8> > r(i.GetRaster());
                            SaveRaster(r);
                        }
                    }
                    else
                    {
                        if(typename T::pixel_format().HasAlpha())
                        {
                            Raster<Pixel<RGBA8> > r (i.GetRaster());
                            SaveRaster(r);
                        }
                        else
                        {
                            Raster<Pixel<RGB8> > r (i.GetRaster());
                            SaveRaster(r);
                        }
                    }
                    
                }
                
        };


    }
}


#endif
