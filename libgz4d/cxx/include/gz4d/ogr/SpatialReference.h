#ifndef GZ4D_OGR_SPATIALREFERENCE_H
#define GZ4D_OGR_SPATIALREFERENCE_H

#include <gz4d/ogr/common.h>
#include <memory>
#include <ogr_spatialref.h>
#include <cpl_string.h>

#include <string>

namespace gz4d
{
    namespace ogr
    {
        class CoordinateTransformation;
        class Layer;
        class Geometry;

        class SpatialReference
        {
            friend class CoordinateTransformation;
            friend class Layer;
            friend class Geometry;

            std::shared_ptr<OGRSpatialReference> spatialReference;
            SpatialReference(OGRSpatialReference * sr);
            SpatialReference(const OGRSpatialReference * sr);
            std::shared_ptr<OGRSpatialReference> const &init();
            public:
                SpatialReference();
                SpatialReference(SpatialReference const &other);
                void reset();

                void SetFromUserInput(std::string const & s);
                void SetWellKnownGeogCS(std::string const & cs);
                void importFromProj4(std::string const & proj4);
                void SetUTM(int zone, bool north=true);
                bool IsSame(SpatialReference const &other) const;
                bool IsGeographic() const;
                std::string exportToProj4() const;

                operator bool() const {return bool(spatialReference);}
        };
    }
}


inline gz4d::ogr::SpatialReference::SpatialReference(OGRSpatialReference * sr)
:spatialReference(make_shared_from_ogr(sr))
{
}

inline gz4d::ogr::SpatialReference::SpatialReference(const OGRSpatialReference * sr)
:spatialReference(sr ? new OGRSpatialReference(*sr) : nullptr)
{
}

inline gz4d::ogr::SpatialReference::SpatialReference()
{
}

inline gz4d::ogr::SpatialReference::SpatialReference(SpatialReference const &other)
:spatialReference(other.spatialReference)
{
}

inline void gz4d::ogr::SpatialReference::reset()
{
    spatialReference.reset();
}

inline std::shared_ptr<OGRSpatialReference> const &gz4d::ogr::SpatialReference::init()
{
    if(!spatialReference)
        spatialReference = std::shared_ptr<OGRSpatialReference>(new OGRSpatialReference,ogr_deleter<OGRSpatialReference>());
    return spatialReference;
}

inline void gz4d::ogr::SpatialReference::SetFromUserInput(std::string const & s)
{
    init()->SetFromUserInput(s.c_str());
}

inline void gz4d::ogr::SpatialReference::SetWellKnownGeogCS(std::string const & cs)
{
    init()->SetWellKnownGeogCS(cs.c_str());
}

inline void gz4d::ogr::SpatialReference::importFromProj4(std::string const & proj4)
{
    init()->importFromProj4(proj4.c_str());
}

inline void gz4d::ogr::SpatialReference::SetUTM(int zone, bool north)
{
    init()->SetUTM(zone,north);
}

inline bool gz4d::ogr::SpatialReference::IsSame(SpatialReference const &other) const
{
    return spatialReference && spatialReference->IsSame(other.spatialReference.get());
}

inline bool gz4d::ogr::SpatialReference::IsGeographic() const
{
    return spatialReference && spatialReference->IsGeographic();
}

inline std::string gz4d::ogr::SpatialReference::exportToProj4() const
{
    if(!spatialReference)
        return "";
    
    char * pname;
    spatialReference->exportToProj4(&pname);
    std::string ret(pname);
    CPLFree(pname);
    return ret;
}


#endif
