#ifndef GZ4D_GEO_ELLIPSOID_H
#define GZ4D_GEO_ELLIPSOID_H

#include <gz4d/gz4d.h>
#include <gz4d/geo/Point.h>
#include <gz4d/geo/ReferenceFrame.h>
#include <gz4d/Angle.h>

namespace gz4d
{
    namespace geo
    {
        template<typename S> class Ellipsoid
        {
/*            double a; ///< Semi-major axis (meters).
            double b; ///< Semi-minor axis (meters).
            double w; ///< anglular velocity (rad/s).

            double e2; ///< Squared eccentricity.*/
            public:
                /// Constructs a reference ellipsoid to represent earth.
                /// Defaults to WGS84.
//                 Ellipsoid(double a = 6378137.0, double b = 6356752.3142 , double w = 7292115e-11):a(a),b(b),w(w)
//                 {
//                     e2 = 1.0-(b*b)/(a*a);
//                 }

                /// Meridional radius of curvature.
                /// Radius of curvature in north-south direction.
                /// @param latitude Latitude in radians.
                static double M(double latitude)
                {
                    return S::a()*(1.0-S::e2())/pow((1.0-S::e2())*pow(sin(latitude),2.0),3.0/2.0);
                }
                
                template<typename RT> static double M(Angle<double,Radian,RT> latitude){return M(latitude.value());}
                template<typename RT> static double M(Angle<double,Degree,RT> latitude){return M(Radians(latitude.value()));}

                /// Transverse radius of curvature.
                /// Radius of curvature in east-west direction.
                /// @param latitude Latitude in radians.
                static double N(double latitude)
                {
                    if(S::e2() == 0.0)
                        return S::a();
                    return S::a()/sqrt(1-S::e2()*pow(sin(latitude),2.0));
                }
                
                template<typename RT> static double N(Angle<double,Radian,RT> latitude){return N(latitude.value());}
                template<typename RT> static double N(Angle<double,Degree,RT> latitude){return N(Radians(latitude.value()));}
                

                template<typename T, typename CF, typename ET> static Point<double,ReferenceFrame<ECEF<>,ET> > ToEarthCenteredEarthFixed( Point<T,ReferenceFrame<Geodetic<CF>,ET> > const &p)
                {
                    double latr = Radians(p[CF::Latitude]);
                    double lonr = Radians(p[CF::Longitude]);
                    double height = p[CF::Height];
                    double n = N(latr);
                    return Point<double,ReferenceFrame<ECEF<>,ET> >((n+height)*cos(latr)*cos(lonr),(n+height)*cos(latr)*sin(lonr),(n*(1.0-S::e2())+height)*sin(latr));
                }

//                 template<typename T> Point<double> ToEarthCenteredEarthFixed(GeodeticPoint<T> const &gp) const
//                 {
//                     return ToEarthCenteredEarthFixed(gp.latitude, gp.longitude, gp.altitude);
//                 }

                template <typename ET> static Point<double,ReferenceFrame<Geodetic<LatLon>,ET> > ToGeodetic(Point<double,ReferenceFrame<ECEF<>,ET> > const &p)
                {
                    Point<double,ReferenceFrame<Geodetic<LatLon>,ET> > ret;

                    double ep2 = (S::a()*S::a())/(S::b()*S::b())-1.0;
                    double r = sqrt(p[0]*p[0]+p[1]*p[1]);
                    double E2 = S::a()*S::a()-S::b()*S::b();
                    double F = 54*S::b()*S::b()*p[2]*p[2];
                    double G = r*r +(1.0-S::e2())*p[2]*p[2]-S::e2()*E2;
                    double C = (S::e2()*S::e2()*F*r*r)/(G*G*G);
                    double s = pow(1.0+C+sqrt(C*C+2*C),1/3.0);
                    double P = F/(3.0*pow((s+(1.0/s)+1.0),2.0)*G*G);
                    double Q = sqrt(1.0+2.0*S::e2()*S::e2()*P);
                    double r0 = (-(P*S::e2()*r)/(1.0+Q))+sqrt((1.0/2.0)*S::a()*S::a()*(1.0+1.0/Q)-((P*(1-S::e2())*p[2]*p[2])/(Q*(1.0+Q)))-(1.0/2.0)*P*r*r);
                    double U = sqrt(pow(r-S::e2()*r0,2.0)+p[2]*p[2]);
                    double V = sqrt(pow(r-S::e2()*r0,2.0)+(1.0-S::e2())*p[2]*p[2]);
                    double Z0 = S::b()*S::b()*p[2]/(S::a()*V);

                    ret[LatLon::Height] = U*(1.0-(S::b()*S::b())/(S::a()*V));
                    ret[LatLon::Latitude] = Degrees(atan((p[2]+ep2*Z0)/r));
                    ret[LatLon::Longitude] = Degrees(atan2(p[1],p[0]));
                    return ret;
                }
        };
    }
}

#endif
