#include "OGR_LineString.h"
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/PushAttrib.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include <gz4d/geo/Projection.h>

extern MainWindow * mainWindow;

OGR_LineString::OGR_LineString(gz4d::ogr::LineString line, gz4d::ogr::SpatialReference spatialRef, double xref, double yref, OGR_Limits const &limits, float minSpan)
{
    gz4d::ogr::SpatialReference sr = spatialRef;
    if(line.getSpatialReference())
        sr = line.getSpatialReference();

    gz4d::geo::Projection proj(sr,mainWindow->getScene3D()->getSpatialReference());

    float minSpanSqr = minSpan*minSpan;
    gz4d::Point<double> lastp;

    for(gz4d::ogr::LineString::const_iterator p = line.begin(); p != line.end(); ++p)
    {
        if(limits(*p))
        {
            gz4d::Point<double> pp = proj(*p);
            bool skip = false;
            if(p != line.begin() && p < line.end() - 1)
            {
                float d = (pp[0]-lastp[0])*(pp[0]-lastp[0])+(pp[1]-lastp[1])*(pp[1]-lastp[1]);
                if (d < minSpanSqr)
                    skip = true;
            }
            if(!skip)
            {
                points.push_back(gz4d::Point<float>(pp[0]-xref,pp[1]-yref,-(*p)[2]));
                lastp = pp;
            }
        }
    }
}

void OGR_LineString::Draw(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib();
    glDisable(GL_LIGHTING);
    glColor3f(1.0,1.0,1.0);
    glBegin(GL_LINE_STRIP);
    for(std::vector<gz4d::Point<float> >::iterator p = points.begin(); p != points.end(); ++p)
    {
        gz4d::glo::Vertex(*p);
    }
    glEnd();
}
