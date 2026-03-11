// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/ogr.h>
#include <gz4d/glut/Application.h>
#include <gz4d/vector/ios.h>
#include <gz4d/glo/Translate.h>
#include <gz4d/render/DrawPolygon.h>
#include <gz4d/data/Triangle.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/vector/operations.h>
#include <set>
#include <gz4d/data/TriangulatedPolygon.h>
#include <gz4d/ogr/link.h>

typedef std::vector<gz4d::Polygon<gz4d::Point<double> > > PolygonList;

class TriangulationViewer: public gz4d::glut::Application<TriangulationViewer,gz4d::glut::Ortho<> >
{
    private:
        PolygonList polygons;
        double scale;
        gz4d::Point<double> offset;

        std::vector<gz4d::TriangulatedPolygon<gz4d::Point<double> > > tPolygons;

        float viewPercentage;
        bool seeking;
    public:
        TriangulationViewer(PolygonList const &p, gz4d::Box2d const &bbox):polygons(p),scale(1.0),viewPercentage(1.0),seeking(false)
        {
            scale = 2.0/bbox.getMaxLength();
            offset = -gz4d::Vector<double,3>(bbox.getCenter(),0);

            for(int i = 0; i < polygons.size(); ++i)
            {
                tPolygons.push_back(gz4d::TriangulatedPolygon<gz4d::Point<double> >(polygons[i]));
            }

        }

        void Draw()
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glPushMatrix();

            glScaled(scale,scale,scale);
            gz4d::glo::Translate(offset);

            for(int p = 0; p < tPolygons.size(); ++p)
            {
                int tCount = tPolygons[p].vertices.size() *viewPercentage;
                gz4d::glo::Color(gz4d::Blue());
                if(tCount)
                    gz4d::DrawArrays(tPolygons[p].vertices,GL_TRIANGLES,0,tCount);
            }

            for(int p = 0; p < tPolygons.size(); ++p)
            {
                gz4d::glo::Color(gz4d::Black());
                gz4d::DrawArrays(polygons[p].getExteriorRing(),GL_LINE_STRIP);
                gz4d::glo::Color(gz4d::Grey());
                for(int i = 0; i < polygons[p].getInteriorRings().size(); ++i)
                    gz4d::DrawArrays(polygons[p].getInteriorRing(i),GL_LINE_STRIP);
            }

//             glBegin(GL_LINE_STRIP);
//             for(int i = 0; i < vCount; ++i)
//                 gz4d::glo::Vertex(polygons[0].exteriorRing[i]);
//             glEnd();

//             for(PolygonList::iterator i = polygons.begin(); i != polygons.end(); ++i)
//                 gz4d::DrawPolygonOutline(*i,gz4d::Green(),gz4d::Red());
//            gz4d::DrawPolygonOutline(polygons[0],gz4d::Green(),gz4d::Red());

            glPopMatrix();
            SwapBuffers();
        }
        
        void Mouse(int button, int state,int x, int y)
        {
            if(button == 0)
            {
                if(state == 0)
                {
                    seeking = true;
                    Motion(x,y);
                }
                else
                {
                    seeking = false;
                }
            }
        }

        void Motion(int x, int y)
        {
            if(seeking)
            {
                viewPercentage = std::min(1.0f,std::max(0.0f,x/float(_window_width)));
                PostRedisplay();
            }
        }

};



int main(int argc, char * argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cerr << "triangulate dataSource [layerName]" << std::endl;
        std::cerr << "    dataSource is a file supported by OGR (GDAL)." << std::endl;
        std::cerr << "    layerName is the name of the layer to triangulate. If omited,\n        layers of the dataSource are listed." << std::endl;
        exit(1);
    }
    gz4d::ogr::DataSource ds(argv[1]);
    //std::cout << ds.GetName() << std::endl;
    if(argc == 2)
    {
        std::cout << "  " << ds.size() << " layers" << std::endl;
        for(int l = 0; l < ds.size(); ++l)
        {
            gz4d::ogr::Layer layer = ds[l];
            std::cout << "    " << layer.GetName() << ": layer with " << layer.size() << " features" << std::endl;
            layer.ResetReading();
            std::map<std::string,int> featureTypes;
            for(gz4d::ogr::Feature feature = layer.GetNextFeature(); feature; feature = layer.GetNextFeature())
            {
                gz4d::ogr::Geometry g = feature.GetGeometry();
                std::string gtype = "None";
                if(g)
                {
                    gtype = g.getName();
                    if(gtype == "POLYGON")
                    {
                        gz4d::ogr::Polygon p = g;
                        if(p.getNumInteriorRings())
                            gtype = "POLYGON (with interior rings)";
                    }
                }

                if(featureTypes.count(gtype)==0)
                    featureTypes[gtype]=1;
                else
                    featureTypes[gtype]+=1;
            }
            for(std::map<std::string,int>::iterator i = featureTypes.begin(); i != featureTypes.end(); ++i)
                std::cout << "      " << i->first << "\t" << i->second << std::endl;
        }
    }
    else
    {
        gz4d::ogr::Layer layer = ds[std::string(argv[2])];
        std::cout << "    " << layer.GetName() << ": layer with " << layer.size() << " features" << std::endl;
        layer.ResetReading();
        PolygonList polygons;
        for(gz4d::ogr::Feature feature = layer.GetNextFeature(); feature; feature = layer.GetNextFeature())
        {
            gz4d::ogr::Polygon p = feature.GetGeometry();
            if(p)
            {
                polygons.push_back(p);
            }
        }
        std::cout << "      " << polygons.size() << " polygons" << std::endl;
        for(PolygonList::iterator p = polygons.begin(); p != polygons.end(); ++p)
        {
            std::cout << "        E: " << p->getExteriorRing().size();
            for(std::vector<std::vector<gz4d::Point<double> > >::const_iterator ir = p->getInteriorRings().begin(); ir != p->getInteriorRings().end(); ++ir)
                std::cout << " I: " << ir->size();
            std::cout << std::endl;
        }
        gz4d::ogr::Envelope e = layer.GetExtent();
        if(e)
        {
            gz4d::Box2d b = e;
            std::cout << "      bbox min: " << b.getMin() << " max: " << b.getMax() << std::endl;
            TriangulationViewer tv(polygons, b);
            tv("Triangulator");
        }
    }
}

