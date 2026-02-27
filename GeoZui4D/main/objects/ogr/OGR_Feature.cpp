#include "OGR_Feature.h"
#include "OGR_Polygon.h"
#include "OGR_LineString.h"
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include <gz4d/geo/Projection.h>

extern MainWindow * mainWindow;


OGR_Feature::OGR_Feature(gz4d::ogr::Feature feature, gz4d::ogr::SpatialReference sr, double xref, double yref, OGR_Limits const &limits, float minSpan)
{
    gz4d::ogr::Geometry geo = feature.GetGeometry();
    if(geo)
    {
        gz4d::ogr::SpatialReference spatialRef = sr;
        if(geo.getSpatialReference())
            spatialRef = geo.getSpatialReference();

        gz4d::geo::Projection proj(spatialRef,mainWindow->getScene3D()->getSpatialReference());

        gz4d::Box<gz4d::Vector<double,2> > b = geo.getEnvelope();
        gz4d::Point<double> min,max;
        min = gz4d::Vector<double,3>(b.getMin(),0);
        max = gz4d::Vector<double,3>(b.getMax(),0);

        min = proj(min);
        max = proj(max);

        if( ((max[0]-min[0])*(max[0]-min[0]) + (max[1]-min[1])*(max[1]-min[1])) >= minSpan*minSpan)
        {
            gz4d::ogr::Polygon p = geo;
            if(p)
                geometry = std::shared_ptr<SimpleObject>(new OGR_Polygon(p,sr,xref,yref, limits, minSpan));
            gz4d::ogr::LineString ls = geo;
            if(ls)
                geometry = std::shared_ptr<SimpleObject>(new OGR_LineString(ls,sr,xref,yref, limits, minSpan));
        }
    }
}

void OGR_Feature::Draw(float degradation, Window3D *win)
{
    if(geometry)
        geometry->Draw(degradation,win);
}