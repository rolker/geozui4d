#ifndef GZ4D_GEO_POSITION_H
#define GZ4D_GEO_POSITION_H

#include <gz4d/gz4d.h>
#include <gz4d/util/Utilities.h>

#include <cmath>
#include <gz4d/Angle.h>

namespace gz4d
{
    namespace geo
    {
        /// Geographic position in degrees.
        template<typename T> struct Position
        {
            typedef Angle<T,Degree,ZeroCenteredPeriod> latitude_type;
            typedef Angle<T,Degree,ZeroCenteredPeriod> longitude_type;
            typedef Angle<T,Degree,ZeroCenteredPeriod> value_type;
            
            
            latitude_type latitude;
            longitude_type longitude;

            Position():latitude(Nan<T>()),longitude(Nan<T>()){}
            Position(latitude_type lat, longitude_type lon):latitude(lat),longitude(lon){}

            static std::size_t size() {return 2;}
            value_type const &operator[](std::size_t i) const
            {
                if (i == 0)
                    return latitude;
                return longitude;
            }

            value_type &operator[](std::size_t i)
            {
                if (i == 0)
                    return latitude;
                return longitude;
            }

            Position &operator+=(value_type val)
            {
                latitude += val;
                longitude += val;
                return *this;
            }

            template <typename OT> Position operator*(OT const &m) const
            {
                return Position<T>(latitude*m, longitude*m);
            }
            
            template <typename OT> Position operator+(Position<OT> const &p) const
            {
                return Position<T>(latitude+p.latitude, longitude+p.longitude);
            }

            bool operator==(const Position &o) const
            {
                return latitude == o.latitude && longitude == o.longitude;
            }
        };

        template <typename PT, typename OT> inline Position<PT> operator*(OT o, Position<PT> const &p)
        {
            return p*o;
        }

    }
}

#endif
