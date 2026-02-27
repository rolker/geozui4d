#include <gz4d/geo/Projection.h>
#include <gz4d/vector/ios.h>
#include <gz4d/ios.h>
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>

int main(int argc, char **argv)
{
    std::vector<std::string> args(argv, argv + argc);
    if(args.size() < 3 || args.size() > 4)
    {
        std::cerr << "usage: project_utm [zone] lat lon\n\tlat and lon are in decimal degrees." << std::endl;
        return 1;
    }
    double lat,lon;
    int zone;
    int lat_idx = 1;
    if(args.size() == 4)
    {
        zone = boost::lexical_cast<int>(args[1]);
        lat_idx = 2;
    }
    lat = boost::lexical_cast<double>(args[lat_idx]);
    lon = boost::lexical_cast<double>(args[lat_idx+1]);
    if(args.size() == 3)
    {
        zone = gz4d::geo::UTMZone(lon);
    }
    bool n = (lat >= 0.0);

    std::cout << "zone: " << zone;
    if (n)
        std::cout << "N" << std::endl;
    else
        std::cout << "S" << std::endl;
    gz4d::geo::Projection p = gz4d::geo::UTMProjection(zone,n);
    gz4d::Point<double> ret = p(lat,lon);
    std::cout << std::setprecision(10) << ret << std::endl;

    gz4d::geo::Position<double> dblcheck = p.inverse(ret[0],ret[1]);
    std::cout << "Lat: " << dblcheck.latitude << ", Lon: " << dblcheck.longitude << std::endl;
}
