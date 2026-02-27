#ifndef GZ4D_GEO_LOCALENU_H
#define GZ4D_GEO_LOCALENU_H

#include <gz4d/gz4d.h>
#include <gz4d/Translation.h>
#include <gz4d/geo/WGS84.h>
#include <gz4d/geo/Point.h>
#include <gz4d/Matrix.h>
#include <vector>
#include <gz4d/matrix/operations.h>
#include <gz4d/Box.h>

namespace gz4d
{
    namespace geo
    {
        template <typename ET=WGS84::Ellipsoid> class LocalENU
        {
            //Point<double, ReferenceFrame<Geodetic<LatLon>, ET> > reference;
            //Point<double, ReferenceFrame<ECEF<>, ET> > refECEF;
            Matrix<double, 4,4> transform;
            Matrix<double, 4,4> inverse;
            public:
                typedef std::shared_ptr<LocalENU<ET> > Ptr;

                LocalENU()
                //:refECEF(reference)
                {
                }

                LocalENU(Point<double, ReferenceFrame<Geodetic<LatLon>, ET> > const &ref)
                //:reference(ref),refECEF(ref)
                {
                    Point<double, ReferenceFrame<ECEF<>, ET> > refECEF(ref);
                    double lat = Radians(ref[0]);
                    double lon = Radians(ref[1]);

                    transform(0,0) = -sin(lon);
                    transform(0,1) = cos(lon);
                    transform(0,2) = 0.0;
                    transform(0,3) = 0.0;
                    transform(1,0) = -sin(lat)*cos(lon);
                    transform(1,1) = -sin(lat)*sin(lon);
                    transform(1,2) = cos(lat);
                    transform(1,3) = 0.0;
                    transform(2,0) = cos(lat)*cos(lon);
                    transform(2,1) = cos(lat)*sin(lon);
                    transform(2,2) = sin(lat);
                    transform(2,3) = 0.0;
                    transform(3,0) = 0.0;
                    transform(3,1) = 0.0;
                    transform(3,2) = 0.0;
                    transform(3,3) = 1.0;
                    inverse = Translation<double>(refECEF).GetMatrix()*transpose(transform);
                    transform = transform*Translation<double>(-refECEF).GetMatrix();

                }

//                 Point<double, ReferenceFrame<Geodetic<LatLon>, ET> > const &getReference() const
//                 {
//                     return reference;
//                 }

//                 Matrix<double,4,4> GetMatrix(Point<double, ReferenceFrame<ECEF<>, ET> > const &p) const
//                 {
//                     Matrix<double,4,4> ret;
//                     for(int i = 0; i < 3; ++i)
//                         for(int j = 0; j < 3; ++j)
//                             ret(i,j) = transform(i,j);
//                     ret(3,3) = 1.0;
//                     return ret*Translation<double>(p-refECEF).GetMatrix();
//                 }

                Matrix<double,4,4> GetMatrix() const
                {
                    return transform;
                }

                Matrix<double,4,4> GetInverseMatrix() const
                {
                    return inverse;
                }

                Point<double, ReferenceFrame<ECEF<>, ET> > toECEF(gz4d::Point<double> const &p) const
                {
                    Vector<double,4> t(p,0);
                    t[3] = 1.0;
                    t = inverse*t;
                    return Point<double, ReferenceFrame<ECEF<>, ET> >(Vector<double,3>(t,0));
                }

                gz4d::Point<double> toLocal(Point<double, ReferenceFrame<ECEF<>, ET> > const &p) const
                {
                    Vector<double,4> t(p,0);
                    t[3] = 1.0;
                    t = transform*t;
                    return Vector<double,3>(t,0);
                }

                std::vector<gz4d::Point<double> > toLocal(std::vector<Point<double, ReferenceFrame<ECEF<>, ET> > > const &pv) const
                {
                    std::vector<gz4d::Point<double> > ret;
                    for(typename std::vector<Point<double, ReferenceFrame<ECEF<>, ET> > >::const_iterator p = pv.begin(); p != pv.end(); ++p)
                        ret.push_back(toLocal(*p));
                    return ret;
                }
                
                Box2d toLonLatBox(const Box2d &local_box) const
                {
                    Point<double, ReferenceFrame<Geodetic<>, ET> > min(toECEF(Vector<double,3>(local_box.getMin(),0)));
                    Point<double, ReferenceFrame<Geodetic<>, ET> > max(toECEF(Vector<double,3>(local_box.getMax(),0)));
                    return Box2d(Vector<double,2>(min[1],min[0]),Vector<double,2>(max[1],max[0]));
                }
        };
    }
}

#endif
