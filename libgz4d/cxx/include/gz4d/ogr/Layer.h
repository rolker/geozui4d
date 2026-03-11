// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_OGR_LAYER_H
#define GZ4D_OGR_LAYER_H

#include <gz4d/ogr/common.h>
#include <memory>
#include <gz4d/ogr/Feature.h>
#include <gz4d/ogr/Envelope.h>
#include <string>

class OGRDataSource;
class OGRLayer;

namespace gz4d
{
    namespace ogr
    {
        class DataSource;
        class SpatialReference;

        class Layer
        {
            friend class DataSource;
            std::shared_ptr<OGRLayer> layer;
            Layer(OGRLayer * l);
            public:
                Layer();
                Layer(Layer const &other);

                void ResetReading() const;
                Feature GetNextFeature() const;
                std::size_t size() const;
                Feature::Definition GetDefinition() const;
                operator bool() const {return bool(layer);}
                SpatialReference GetSpatialRef() const;
                Envelope GetExtent(bool force=true) const;

                std::string GetName() const;
        };
    }
}

#include <ogrsf_frmts.h>
#include <gz4d/ogr/SpatialReference.h>

inline gz4d::ogr::Layer::Layer(OGRLayer * l)
:layer(make_shared_from_ogr(l))
{
}

inline gz4d::ogr::Layer::Layer()
{
}

inline gz4d::ogr::Layer::Layer(Layer const &other)
:layer(other.layer)
{
}

inline void gz4d::ogr::Layer::ResetReading() const
{
    layer->ResetReading();
}

inline gz4d::ogr::Feature gz4d::ogr::Layer::GetNextFeature() const
{
    return Feature(layer->GetNextFeature());
}

inline gz4d::ogr::Feature::Definition gz4d::ogr::Layer::GetDefinition() const
{
    return Feature::Definition(layer->GetLayerDefn());
}

inline std::size_t gz4d::ogr::Layer::size() const
{
    return layer->GetFeatureCount();
}

inline gz4d::ogr::SpatialReference gz4d::ogr::Layer::GetSpatialRef() const
{
    return SpatialReference(layer->GetSpatialRef());
}

inline gz4d::ogr::Envelope gz4d::ogr::Layer::GetExtent(bool force) const
{
    Envelope ret;
    if(layer->GetExtent(ret.envelope.get(),force) == OGRERR_NONE)
        return ret;
    return Envelope();
}

inline std::string gz4d::ogr::Layer::GetName() const
{
    #if GDAL_VERSION_NUM >= 1800
        return layer->GetName();
    #else
        return layer->GetLayerDefn()->GetName();
    #endif
}

#endif
