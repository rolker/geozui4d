// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_OGR_DATASOURCE_H
#define GZ4D_OGR_DATASOURCE_H

#include <gz4d/ogr/common.h>
#include <memory>
#include <string>
#include <gz4d/ogr/Layer.h>
#include <boost/thread/mutex.hpp>
#include <ogrsf_frmts.h>

namespace gz4d
{
    namespace ogr
    {
        class DataSource
        {
            static boost::mutex registrarMutex;
            //std::shared_ptr<OGRDataSource> source;
            std::shared_ptr<GDALDataset> source;

            public:
                DataSource(){}
                DataSource(std::string const &fn)
                {
                    //boost::mutex::scoped_lock lock(Dummy<>::registrarMutex);
                    boost::mutex::scoped_lock lock(registrarMutex);
                    //OGRRegisterAll();
                    GDALAllRegister();
                    //source = std::shared_ptr<OGRDataSource>(OGRSFDriverRegistrar::Open(fn.c_str()),OGRDataSource::DestroyDataSource);
                    source = std::shared_ptr<GDALDataset>((GDALDataset*)GDALOpenEx(fn.c_str(),GDAL_OF_VECTOR, NULL, NULL, NULL),GDALClose);
                }

                DataSource(DataSource const &other)
                :source(other.source)
                {
                }

                Layer operator[](int i)
                {
                    return Layer(source->GetLayer(i));
                }

                Layer operator[](std::string const &name)
                {
                    return Layer(source->GetLayerByName(name.c_str()));
                }

                std::size_t size() const
                {
                    return source->GetLayerCount();
                }

//                 std::string GetName() const
//                 {
//                     return source->GetName();
//                 }

                operator bool() const {return bool(source);}

        };
    }
}

#endif
