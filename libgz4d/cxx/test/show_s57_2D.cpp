#include <gz4d/ogr.h>
#include <gz4d/glut/Application.h>
#include <gz4d/vector/ios.h>
#include <gz4d/glo/Translate.h>
#include <gz4d/render/DrawPolygon.h>
#include <gz4d/render/DrawArrays.h>
#include <gz4d/data/Triangle.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/vector/operations.h>
#include <gz4d/ogr/link.h>
#include <set>

typedef std::vector<gz4d::Polygon<gz4d::Point<double> > > PolygonList;

typedef std::vector<gz4d::Point<double> > LineType;
typedef std::vector<LineType> LineList;

class TriangulationViewer: public gz4d::glut::Application<TriangulationViewer,gz4d::glut::Ortho<> >
{
    private:
        PolygonList land_polygons;
        PolygonList sea_polygons;
        LineList coastlines;

    public:
        TriangulationViewer(std::string const &fname)
        {
            InitDisplayMode(GLUT_DOUBLE|GLUT_STENCIL);
            
            gz4d::ogr::DataSource ds(fname);
            //std::cout << ds.GetName() << std::endl;
            
            gz4d::ogr::Layer land_layer = ds[std::string("LNDARE")];
            for(gz4d::ogr::Feature feature = land_layer.GetNextFeature(); feature; feature = land_layer.GetNextFeature())
            {
                gz4d::ogr::Polygon p = feature.GetGeometry();
                if(p)
                {
                    land_polygons.push_back(p);
                }
            }
            std::cout << "      " << land_polygons.size() << " land polygons" << std::endl;
            gz4d::ogr::Envelope le = land_layer.GetExtent();
            
            gz4d::ogr::Layer sea_layer = ds[std::string("DEPARE")];
            for(gz4d::ogr::Feature feature = sea_layer.GetNextFeature(); feature; feature = sea_layer.GetNextFeature())
            {
                gz4d::ogr::Polygon p = feature.GetGeometry();
                if(p)
                {
                    sea_polygons.push_back(p);
                }
            }
            std::cout << "      " << sea_polygons.size() << " sea polygons" << std::endl;
            gz4d::ogr::Envelope se = sea_layer.GetExtent();

            gz4d::ogr::Layer coast_layer = ds[std::string("COALNE")];
            for(gz4d::ogr::Feature feature = coast_layer.GetNextFeature(); feature; feature = coast_layer.GetNextFeature())
            {
                gz4d::ogr::LineString ls = feature.GetGeometry();
                if(ls)
                    coastlines.push_back(LineType(ls.begin(),ls.end()));
            }

            std::cout << "      " << coastlines.size() << " coastline segments" << std::endl;
            gz4d::ogr::Envelope cle = coast_layer.GetExtent();
            
            
            if(le && se)
            {
                gz4d::Box2d lb = le;
                gz4d::Box2d sb = se;
                gz4d::Box2d b = lb;
                b.expand(sb);
                
                std::cout << "      bbox min: " << b.getMin() << " max: " << b.getMax() << std::endl;
                
                _scale = 2.0/b.getMaxLength();
                _offset_x = -b.getCenter()[0];
                _offset_y = -b.getCenter()[1];
                
            }
            
        }

        void Draw()
        {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            glPushMatrix();

            gz4d::glo::Color(gz4d::Green());
            gz4d::DrawPolygons(land_polygons);
            
            gz4d::glo::Color(gz4d::Black());
            for(LineList::iterator l = coastlines.begin(); l != coastlines.end(); ++l)
                DrawArrays(*l,GL_LINE_STRIP);

            gz4d::glo::Color(gz4d::Blue());
            gz4d::DrawPolygons(sea_polygons);

            glPopMatrix();
            SwapBuffers();
        }

};



int main(int argc, char * argv[])
{
    if (argc != 2)
    {
        std::cerr << "show_s57_2D enc" << std::endl;
        exit(1);
    }
    
    TriangulationViewer tv(argv[1]);
    tv("S57");
    

}

