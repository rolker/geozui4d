#ifndef GZ4D_S57_S57_H
#define GZ4D_S57_S57_H

#include <gz4d/gz4d.h>
#include <gz4d/Vector.h>
#include <gz4d/Box.h>
#include <gz4d/s57/Cell.h>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>

namespace gz4d
{
    namespace s57
    {
        class Cell;

        class S57
        {
            Cell::List cells;
            Box3f bbox;
            public:
                typedef std::shared_ptr<S57> Ptr;
                S57(std::string const &catalog);

                /// Find charts around a point of view (pov) with radius of view r.
                /// p0 is pixel size at near plane and dp is rate of change of pixels size with distance.
                Cell::List intersects(geo::Point<double,geo::WGS84::ECEF> const &pov, double r, Vector<float,2> const &p0, Vector<float,2> const &dp);
                Box3f const &getBBox() const {return bbox;}
        };
    }
}


inline gz4d::s57::S57::S57(std::string const &catalog_filename)
{
    boost::property_tree::ptree catalog;
    read_xml(catalog_filename,catalog);
    
    boost::property_tree::ptree::const_iterator n = catalog.begin();
    for(boost::property_tree::ptree::const_iterator cn = n->second.begin(); cn != n->second.end(); ++cn)
    {
        if(cn->first == "cell")
        {
            Cell::Ptr c(new Cell(cn->second, std::filesystem::path(catalog_filename).parent_path().string()));
            bbox.expand(c->getBBox());
            cells.push_back(c);
        }
    }
    
    for(std::vector<Cell::Ptr>::iterator c = cells.begin(); c != cells.end(); ++c)
    {
        Box3f cbbox = (*c)->getBBox();
        bool hasContainer = false;
        for(std::vector<Cell::Ptr>::iterator pp = cells.begin(); !hasContainer && pp != cells.end(); ++pp)
        {
            if(pp != c)
            {
                if((*pp)->getBBox().contains(cbbox))
                    hasContainer = true;
            }
        }
        if(!hasContainer)
            (*c)->setTopLevel(true);
    }
}

inline gz4d::s57::Cell::List gz4d::s57::S57::intersects(geo::Point<double,geo::WGS84::ECEF> const &pov, double r, Vector<float,2> const &p0, Vector<float,2> const &dp)
{
    Cell::List ret;
    for(std::vector<Cell::Ptr>::iterator c = cells.begin(); c != cells.end(); ++c)
    {
        double d = (*c)->getECEFBBox().distance(pov);
        if(d < r)
        {
            Vector<float,2> pixelSize = p0 + dp*d;
            float chartPixelSize = (*c)->getScale() * 0.000338667; // meters per dot at 75 dpi
            if(std::min(pixelSize[0],pixelSize[1]) < chartPixelSize)
                ret.push_back(*c);
        }
    }
    return ret;
}



#endif
