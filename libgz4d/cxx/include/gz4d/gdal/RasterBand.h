// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GDAL_RASTERBAND_H
#define GZ4D_GDAL_RASTERBAND_H

#include <gz4d/gz4d.h>
#include <gdal_priv.h>

namespace gz4d
{
    namespace gdal
    {
        class Dataset;
        class RasterBand
        {
            friend class Dataset;
            
            std::shared_ptr<GDALDataset> _dataset;
            GDALRasterBand * _band;
            
            RasterBand(std::shared_ptr<GDALDataset> ds, int n)
            :_dataset(ds),_band(_dataset->GetRasterBand(n))
            {}

        public:
            RasterBand(RasterBand const &o)
            :_dataset(o._dataset),_band(o._band)
            {}
            
            std::vector<std::string> getMetadata() const
            {
                std::vector<std::string> ret;
                char **meta = _band->GetMetadata();
                if(meta)
                    for (int i = 0; meta[i]; ++i)
                        ret.push_back(meta[i]);
                return ret;
            }
            
            std::string getMetadata(std::string const &item) const
            {
                std::string ret = _band->GetMetadataItem(item.c_str());
                return ret;
            }
            
            template <typename T> typename gz4d::Raster<T>::Ptr getRaster() const
            {
                typename gz4d::Raster<T>::Ptr ret(new gz4d::Raster<T>(_dataset->GetRasterXSize(),_dataset->GetRasterYSize(),0));
                _band->RasterIO(GF_Read,0,0,_dataset->GetRasterXSize(),_dataset->GetRasterYSize(),ret->raw(),ret->GetColumns(),ret->GetRows(),_band->GetRasterDataType(),0,0);
                return ret;
            }
            
            double getMin() const
            {
                return _band->GetMinimum();
            }
            
            double getMax() const
            {
                return _band->GetMaximum();
            }
            
            
            double getNullValue() const
            {
                return _band->GetNoDataValue();
            }
            
        };
            
    }
}

#endif
