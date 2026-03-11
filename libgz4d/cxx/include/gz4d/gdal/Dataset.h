// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GDAL_DATASET_H
#define GZ4D_GDAL_DATASET_H

//gz4d_cmake depends GDAL

#include <gz4d/gz4d.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <memory>
#include <sstream>
#include <gz4d/Image.h>
#include <gz4d/Box.h>
#include "RasterBand.h"
#include <gz4d/geo/Projection.h>

namespace gz4d
{
    namespace gdal
    {
        class Dataset
        {
            std::shared_ptr<GDALDataset> _dataset;

        public:
            Dataset(){}
            Dataset(std::string const &fname)
            {
                GDALAllRegister();
                _dataset = std::shared_ptr<GDALDataset>( reinterpret_cast<GDALDataset*>(GDALOpen(fname.c_str(),GA_ReadOnly)),GDALClose);
            }
            Dataset(Dataset const &ds):_dataset(ds._dataset){}

            operator bool() const {return bool(_dataset);}

            std::string info() const
            {
                std::stringstream ss;
                if(_dataset)
                {
                    ss << "size: " << _dataset->GetRasterXSize() << "x" << _dataset->GetRasterYSize() << ", " << _dataset->GetRasterCount() << " bands";
                }
                else
                    ss << "empty";
                return ss.str();
            }

            ImageRGBA8 getImage() const
            {
                std::size_t width = _dataset->GetRasterXSize();
                std::size_t height = _dataset->GetRasterYSize();

                ImageRGBA8 ret(width,height,Pixel<RGBA8>(Black()));
                
                if(_dataset->RasterIO(GF_Read,0,0,width,height,ret.raw()+ret.width()*4*(ret.height()-1),width,height,GDT_Byte,4,NULL,4,-4*width,1) != CE_None)
                    std::cerr << "gdal dataset: error reading image" << std::endl;
                
                
                return ret;
            }

            ImageRGBA8 getImage(std::size_t maxWidth, std::size_t maxHeight) const
            {
                if(_dataset->GetRasterXSize() <= maxWidth && _dataset->GetRasterYSize() <= maxHeight)
                    return getImage();
                double factor = std::min(maxWidth/double(_dataset->GetRasterXSize()),maxHeight/double(_dataset->GetRasterYSize()));
                std::size_t destWidth = _dataset->GetRasterXSize()*factor;
                std::size_t destHeight = _dataset->GetRasterYSize()*factor;
                ImageRGBA8 ret(destWidth,destHeight,Pixel<RGBA8>(Black()));

                if(_dataset->GetRasterCount() == 1 && _dataset->GetRasterBand(1)->GetColorInterpretation() == GCI_PaletteIndex)
                {
                    Raster<uint8_t> r(destWidth,destHeight,0);
                    if(_dataset->GetRasterBand(1)->RasterIO(GF_Read,0,0,_dataset->GetRasterXSize(),_dataset->GetRasterYSize(),r.raw(),destWidth,destHeight,GDT_Byte,0,0) != CE_None)
                        std::cerr << "gdal dataset: error reading image" << std::endl;

                    GDALColorTable * ct = _dataset->GetRasterBand(1)->GetColorTable();
                    for(std::size_t y = 0; y < destHeight; ++y)
                        for(std::size_t x = 0; x < destWidth; ++x)
                        {
                            GDALColorEntry const *ce = ct->GetColorEntry(r.GetRow(destHeight-1-y)[x]);
                            ret.GetRaster().GetRow(y)[x][0] = ce->c1;
                            ret.GetRaster().GetRow(y)[x][1] = ce->c2;
                            ret.GetRaster().GetRow(y)[x][2] = ce->c3;
                            ret.GetRaster().GetRow(y)[x][3] = ce->c4;
                        }
                }
                else
                {
                    if(_dataset->RasterIO(GF_Read,0,0,_dataset->GetRasterXSize(),_dataset->GetRasterYSize(),ret.GetRaster().raw(),destWidth,destHeight,GDT_Byte,4,NULL,4,4*destWidth,1) != CE_None)
                        std::cerr << "gdal dataset: error reading image" << std::endl;
                }
                    
                
                return ret;
            }

            Vector<std::size_t,2> getSize() const
            {
                return Vector<std::size_t,2>(_dataset->GetRasterXSize(),_dataset->GetRasterYSize());
            }
            
            std::size_t getBandCount() const
            {
                return _dataset->GetRasterCount();
            }
            
            RasterBand getBand(std::size_t n) const
            {
                return RasterBand(_dataset,n);
            }
            
            
            Box<Vector<double,2> > getBBox() const
            {
                Box<Vector<double,2> > ret;
                double affine[6];
                _dataset->GetGeoTransform(affine);
                for (int i = 0; i < 2; ++i)
                    for(int j = 0; j < 2; ++ j)
                    {
                        double x = affine[0] + i*_dataset->GetRasterXSize()*affine[1]+j*_dataset->GetRasterYSize()*affine[2];
                        double y = affine[3] + i*_dataset->GetRasterXSize()*affine[4]+j*_dataset->GetRasterYSize()*affine[5];
                        ret.expand(Vector<double,2>(x,y));
                    }
                return ret;
            }
            
            Vector<double,2> pixelSize() const
            {
                double affine[6];
                _dataset->GetGeoTransform(affine);
                return Vector<double,2>(affine[1],affine[5]);
            }

            Vector<double,2> origin() const
            {
                double affine[6];
                _dataset->GetGeoTransform(affine);
                return Vector<double,2>(affine[0],affine[3]);
            }
            
            std::vector<std::string> getMetadata() const
            {
                std::vector<std::string> ret;
                char **meta = _dataset->GetMetadata();
                if(meta)
                    for (int i = 0; meta[i]; ++i)
                        ret.push_back(meta[i]);
                return ret;
            }

            std::string getMetadata(std::size_t n, std::string const &item) const
            {
                std::string ret = _dataset->GetMetadataItem(item.c_str());
                return ret;
            }
            
            geo::Projection getProjection() const
            {
                return geo::Projection(_dataset->GetProjectionRef());
            }
            
        };
    }
}

#endif
