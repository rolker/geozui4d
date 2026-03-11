// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_PROJECTION_H
#define GZ4D_GEO_PROJECTION_H

#include <gz4d/gz4d.h>
#include <gz4d/geo.h>
#include <gz4d/Point.h>
//#include <gz4d/geo/GeodeticPoint.h>
#include <gz4d/geo/WGS84.h>
#include <memory>
#include <gz4d/ogr/CoordinateTransformation.h>
#include <gz4d/util/Utilities.h>
#include <gz4d/Exception.h>
#include <gz4d/nav/Fix.h>

#include <ogr_spatialref.h>

#include <string>
#include <boost/lexical_cast.hpp>

namespace gz4d
{
    namespace geo
    {
        class Projection
        {
            ogr::CoordinateTransformation proj;
            ogr::CoordinateTransformation iproj;
            std::string params;
            public:
                inline Projection(){}
                
                inline Projection(std::string const &p)
                :params(p)
                {
                    ogr::SpatialReference src,dst;
                    src.SetWellKnownGeogCS("WGS84");
                    dst.SetFromUserInput(p);
                    proj = ogr::CoordinateTransformation(src,dst);
                    iproj = ogr::CoordinateTransformation(dst,src);
                }
                
                inline Projection(ogr::SpatialReference const &from, ogr::SpatialReference const &to)
                {
                    proj = ogr::CoordinateTransformation(from,to);
                    iproj = ogr::CoordinateTransformation(to,from);
                }
                
                inline gz4d::Point<double> operator()(double lat, double lon, double alt = 0.0) const
                {
                    proj.Transform(1,&lon,&lat,&alt);
                    return gz4d::Point<double>(lon,lat,alt);
                }
                
                inline gz4d::Point<double> operator()(Angle<double,Degree,ZeroCenteredPeriod> lat, Angle<double,Degree,ZeroCenteredPeriod> lon, double alt=0.0) const
                {
                    return operator()(lat.value(),lon.value(),alt);
                }
                
                gz4d::Point<double> operator()(nav::Fix const &fix) const
                {
                    return operator()(fix.latitude,fix.longitude);
                }
                
                inline gz4d::Point<double> operator()(gz4d::Point<double> const &p) const
                {
                    return operator()(p[1],p[0],p[2]);
                }
                
                template <typename T> inline gz4d::Point<double> operator()(Position<T> const &p) const
                {
                    return operator()(p.latitude, p.longitude);
                }
                template <typename T> inline gz4d::Point<double> operator()(GeodeticPoint<T> const &p) const
                {
                    return operator()(p.latitude.value(), p.longitude.value(), p.altitude);
                }

                inline Position<double> inverse(double x, double y) const
                {
                    iproj.Transform(1,&x,&y);
                    return Position<double>(y,x);
                }
                
                Position<double> inverse(const Vector<double,2> &xy) const
                {
                    return inverse(xy[0],xy[1]);
                }

                GeodeticPoint<double> inverse(gz4d::Point<double> p) const
                {
                    iproj.Transform(1,&p[0],&p[1],&p[2]);
                    return GeodeticPoint<double>(p[1],p[0],p[2]);
                }
                
                inline operator bool() const {return proj;}
                inline bool operator==(Projection const &other) {return params == other.params;}

                std::string const &getParameters() const
                {
                    return params;
                }
        };

        inline int UTMZone(double lon)
        {
            int ret = (int)ceil((lon + 180.0f) / 6.0f);
            
            if(ret<1)
                ret = 1;
            if(ret>60)
                ret = 60;
            
            return ret;
        }
        
        inline Projection UTMProjection(int zone, bool north=true)
        {
            std::string proj = std::string("+proj=utm +ellps=WGS84 +zone=")+boost::lexical_cast<std::string>(zone);
            if(!north)
                proj += " +south";
            return Projection(proj);
        }

        template <class charT, class traits> inline std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& str, Projection const &p)
        {
            if (p)
                return str << p.getParameters();
            return str << "(none)";
        }

    }
}

#endif
