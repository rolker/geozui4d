// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_S57_CELL_H
#define GZ4D_S57_CELL_H

#include <gz4d/gz4d.h>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>

#include <gz4d/ogr/DataSource.h>
#include <gz4d/Vector.h>
#include <gz4d/Box.h>
#include <gz4d/data/Polygon.h>
#include <gz4d/Point.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <gz4d/render/TessellatedPolygon.h>
#include <gz4d/geo/Point.h>
#include <gz4d/geo/WGS84.h>
//#include <gz4d/geo/LocalENU.h>
#include <gz4d/vector/ios.h>
#include <gz4d/ogr/Polygon.h>
#include <gz4d/ogr/LinearRing.h>

#include <map>
#include <vector>

namespace gz4d
{
    namespace s57
    {
        class S57;

        class Cell
        {
            friend class S57;


            std::string dataSourcePath;
            float cscale;
            std::string name;
            typedef std::map<uint8_t,Box3f> PanelMap;
            PanelMap panels;
            Box3f bbox;
            Box3d ecef_bbox;
            bool topLevel;

            geo::LocalENU<> geoReference;

            void setTopLevel(bool tl=true){topLevel = tl;}

            void LoadThread();
            void LaunchLoadThread();
            boost::thread loadThread;
            boost::mutex loadMutex;

            static boost::mutex pendingMutex;
            static int pending;
            static int maxPending;

            public:
                typedef std::shared_ptr<Cell> Ptr;
                typedef std::vector<std::shared_ptr<Cell> > List;

                struct Data
                {
                    typedef std::shared_ptr<Data> Ptr;

                    typedef std::pair<double,double> DepthRange;
                    typedef std::vector<gz4d::Polygon<gz4d::Point<double> > > PolygonList;

                    typedef TessellatedPolygon<geo::Point<double,geo::WGS84::ECEF> > ECEFPoly;
                    typedef std::map<DepthRange,ECEFPoly > DepthAreaMap;

                    Data(std::string const &path, geo::LocalENU<> const &);

                    PolygonList LoadPolygons(gz4d::ogr::Layer const &layer) const;
                    std::vector<gz4d::geo::Point<double,geo::WGS84::ECEF> > LoadPoints(gz4d::ogr::Layer const &layer) const;

                    geo::LocalENU<> geoReference;
                    ECEFPoly land_area;
                    DepthAreaMap depth_areas;
                    ECEFPoly dredged_area;
                    ECEFPoly dumping_ground;
                    ECEFPoly coverage;
                    ECEFPoly rivers;
                    ECEFPoly lake_area;

                    std::vector<std::vector<gz4d::geo::Point<double,geo::WGS84::ECEF> > > roadway;

                    std::vector<geo::Point<double,geo::WGS84::ECEF> > buoys;
                    std::vector<geo::Point<double,geo::WGS84::ECEF> > beacons;
                };


                Cell(std::string const &file);
                Cell(const boost::property_tree::ptree &node, std::string const &path);
                ~Cell();
                
                bool contains(Vector<float,3> const &p) const;
                bool intersects(Box3f const &b) const;
                std::string info();
                Box3f const &getBBox() const {return bbox;}
                Box3d const &getECEFBBox() const {return ecef_bbox;}

                float getScale() const {return cscale;}
                bool isTopLevel() const {return topLevel;}

                Data::Ptr Load();

            private:
                Data::Ptr data;
        };
    }
}

boost::mutex gz4d::s57::Cell::pendingMutex;
int gz4d::s57::Cell::pending = 0;
int gz4d::s57::Cell::maxPending = 25;

inline gz4d::s57::Cell::Cell(std::string const &file)
:dataSourcePath(file),cscale(-1),topLevel(false)
{
}

inline gz4d::s57::Cell::Cell(const boost::property_tree::ptree &node, std::string const &path)
:topLevel(false)
{
    name = node.get("name",name);
    dataSourcePath = (std::filesystem::path(path)/name/std::string(name+".000")).string();

    cscale = node.get("cscale",cscale);
    
    for(boost::property_tree::ptree::const_iterator n = node.begin(); n != node.end(); ++n)
    {
        if(n->first == "cov")
        {
            uint8_t pno = 0;
            for(boost::property_tree::ptree::const_iterator p = n->second.begin(); p != n->second.end(); ++p)
            {
                if(p->first == "panel")
                {
                    for(boost::property_tree::ptree::const_iterator v = p->second.begin(); v != p->second.end(); ++v)
                    {
                        if(v->first == "panel_no")
                            pno=boost::lexical_cast<uint8_t>(v->second.data());
                        if(v->first == "panel")
                            pno = v->second.get("panel_no",pno);
                        if(v->first == "vertex")
                        {
                            Vector<float,3> pos(0.0);
                            pos[0] = v->second.get("lat",pos[0]);
                            pos[1] = v->second.get("long",pos[1]);
                            panels[pno].expand(pos);
                        }
                    }
                }
            }
            break;
        }
    }
    
    for(PanelMap::iterator i = panels.begin(); i!=panels.end(); ++i)
        bbox.expand(i->second);
    geoReference = geo::LocalENU<>(geo::Point<double, geo::WGS84::LatLon>(Point<double>(bbox.getCenter())));
    
    ecef_bbox.expand(geo::Point<double, geo::WGS84::ECEF>(geo::Point<double, geo::WGS84::LatLon>(Point<double>(bbox.getMin()))));
    ecef_bbox.expand(geo::Point<double, geo::WGS84::ECEF>(geo::Point<double, geo::WGS84::LatLon>(Point<double>(bbox.getMax()))));
}

inline gz4d::s57::Cell::~Cell()
{
    if(loadThread.joinable())
        loadThread.join();
}

inline bool gz4d::s57::Cell::contains(gz4d::Vector<float,3> const &p) const
{
    for(PanelMap::const_iterator i = panels.begin(); i!=panels.end(); ++i)
        if(i->second.contains(p))
            return true;
        return false;
}

inline bool gz4d::s57::Cell::intersects(Box3f const &b) const
{
    for(PanelMap::const_iterator i = panels.begin(); i!=panels.end(); ++i)
        if(i->second.intersects(b))
            return true;
        return false;
}

inline std::string gz4d::s57::Cell::info()
{
    std::stringstream ret;
    ret << name << "\t" << cscale << "\t" << bbox.getMin() << " - " << bbox.getMax();
    return ret.str();
}

inline std::shared_ptr<gz4d::s57::Cell::Data> gz4d::s57::Cell::Load()
{
    std::shared_ptr<gz4d::s57::Cell::Data> ret;
    boost::mutex::scoped_lock lock(loadMutex);
    if(data)
    {
        ret = data;
        data.reset();
        loadThread.join();
        boost::mutex::scoped_lock lock(pendingMutex);
        if(pending > 0)
            --pending;
    }
    else
        LaunchLoadThread();
    return ret;
}

inline void gz4d::s57::Cell::LaunchLoadThread()
{
    if(!loadThread.joinable())
        try
        {
            boost::mutex::scoped_lock lock(pendingMutex);
            if(pending < maxPending)
            {
                loadThread = boost::thread(boost::bind(&Cell::LoadThread,this));
                ++pending;
            }
        }
        catch(boost::thread_resource_error)
        {
        }
}

inline void gz4d::s57::Cell::LoadThread()
{
    std::shared_ptr<gz4d::s57::Cell::Data> d(new Data(dataSourcePath,geoReference));
    boost::mutex::scoped_lock lock(loadMutex);
    data = d;
}

inline gz4d::s57::Cell::Data::PolygonList gz4d::s57::Cell::Data::LoadPolygons(gz4d::ogr::Layer const &layer) const
{
    PolygonList ret;
    if(layer)
    {
        layer.ResetReading();
        
        for (int f = 0; f < layer.size(); ++f)
        {
            gz4d::ogr::Feature feature = layer.GetNextFeature();
            
            gz4d::ogr::Polygon polygon = feature.GetGeometry();
            if(polygon)
                ret.push_back(polygon);
        }
    }
    return ret;
}

inline std::vector<gz4d::geo::Point<double, gz4d::geo::WGS84::ECEF> > gz4d::s57::Cell::Data::LoadPoints(gz4d::ogr::Layer const &layer) const
{
    std::vector<geo::Point<double,geo::WGS84::ECEF> > ret;
    if(layer)
    {
        layer.ResetReading();
        
        for (int f = 0; f < layer.size(); ++f)
        {
            gz4d::ogr::Feature feature = layer.GetNextFeature();
            
            gz4d::ogr::Point point = feature.GetGeometry();
            if(point)
                ret.push_back(geo::Point<double,geo::WGS84::ECEF>(geo::Point<double,geo::WGS84::LonLat>(point)));
        }
    }
    return ret;
}

inline gz4d::s57::Cell::Data::Data(std::string const &path, geo::LocalENU<> const &geoRef)
:geoReference(geoRef)
{
    ogr::DataSource dataSource(path);
    
    PolygonTessellator<geo::Point<double,geo::WGS84::LonLat> > pt;
    
    land_area = pt(LoadPolygons(dataSource[std::string("LNDARE")]));
    dredged_area = pt(LoadPolygons(dataSource[std::string("DRGARE")]));
    dumping_ground = pt(LoadPolygons(dataSource[std::string("DMPGRD")]));
    coverage = pt(LoadPolygons(dataSource[std::string("M_COVR")]));
    rivers = pt(LoadPolygons(dataSource[std::string("RIVERS")]));
    lake_area = pt(LoadPolygons(dataSource[std::string("LAKARE")]));
    
    gz4d::ogr::Layer layer = dataSource[std::string("DEPARE")];
    if(layer)
    {
        layer.ResetReading();
        std::map<DepthRange,PolygonList> tmp;
        for (int f = 0; f < layer.size(); ++f)
        {
            gz4d::ogr::Feature feature = layer.GetNextFeature();
            if(feature["DRVAL1"])
            {
                DepthRange d;
                d.first = feature["DRVAL1"].GetValue<double>();
                if(feature["DRVAL2"])
                    d.second = feature["DRVAL2"].GetValue<double>();
                gz4d::ogr::Polygon polygon = feature.GetGeometry();
                if(polygon)
                    tmp[d].push_back(polygon);
                //depth_areas[d].push_back(polygon);
            }
        }
        for(std::map<DepthRange,PolygonList>::iterator i = tmp.begin(); i != tmp.end(); ++i)
        {
            depth_areas[i->first] = pt(i->second);
        }
    }
    
    buoys = LoadPoints(dataSource[std::string("BOYCAR")]);
    std::vector<geo::Point<double,geo::WGS84::ECEF> > tmp;
    tmp = LoadPoints(dataSource[std::string("BOYINB")]);
    buoys.insert(buoys.end(),tmp.begin(),tmp.end());
    tmp = LoadPoints(dataSource[std::string("BOYISD")]);
    buoys.insert(buoys.end(),tmp.begin(),tmp.end());
    tmp = LoadPoints(dataSource[std::string("BOYLAT")]);
    buoys.insert(buoys.end(),tmp.begin(),tmp.end());
    tmp = LoadPoints(dataSource[std::string("BOYSAW")]);
    buoys.insert(buoys.end(),tmp.begin(),tmp.end());
    tmp = LoadPoints(dataSource[std::string("BOYSPP")]);
    buoys.insert(buoys.end(),tmp.begin(),tmp.end());
    
    beacons = LoadPoints(dataSource[std::string("BCNCAR")]);
    tmp = LoadPoints(dataSource[std::string("BCNISD")]);
    beacons.insert(beacons.end(),tmp.begin(),tmp.end());
    tmp = LoadPoints(dataSource[std::string("BCNLAT")]);
    beacons.insert(beacons.end(),tmp.begin(),tmp.end());
    tmp = LoadPoints(dataSource[std::string("BCNSAW")]);
    beacons.insert(beacons.end(),tmp.begin(),tmp.end());
    tmp = LoadPoints(dataSource[std::string("BCNSPP")]);
    beacons.insert(beacons.end(),tmp.begin(),tmp.end());
    
    layer = dataSource[std::string("ROADWY")];
    if(layer)
    {
        layer.ResetReading();
        for (int f = 0; f < layer.size(); ++f)
        {
            gz4d::ogr::Feature feature = layer.GetNextFeature();
            
            gz4d::ogr::LineString line = feature.GetGeometry();
            if(line)
            {
                roadway.push_back(std::vector<geo::Point<double,geo::WGS84::ECEF> >());
                for(ogr::LineString::const_iterator p = line.begin(); p!= line.end(); ++p)
                    roadway.back().push_back(geo::Point<double,geo::WGS84::ECEF>( geo::Point<double,geo::WGS84::LonLat>(*p)));
            }
        }
    }
}


#endif
