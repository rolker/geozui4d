#include <gz4d/ogr.h>
#include <gz4d/glut/Application.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/vector/ios.h>
#include <gz4d/render/DrawArrays.h>
#include <gz4d/ogr/link.h>

class MyScene: public gz4d::glut::Application<MyScene,gz4d::glut::Ortho<> >
{
    gz4d::Box< gz4d::Vector<double,2> > bbox;
    std::vector<std::vector<gz4d::Point<double> > > rings;
    std::vector<gz4d::Point<double> > points;
    public:
        MyScene(std::string const &f)
        {
            gz4d::ogr::DataSource ds(f);
            gz4d::ogr::Envelope envelope;
            for(int l = 0; l < ds.size(); ++l)
            {
                gz4d::ogr::Layer layer = ds[l];
                envelope.Merge(layer.GetExtent());
                layer.ResetReading();
                for (int f = 0; f < layer.size(); ++f)
                {
                    gz4d::ogr::Feature feature = layer.GetNextFeature();

                    gz4d::ogr::Point point = feature.GetGeometry();
                    if(point)
                    {
                        points.push_back(point);
                    }
                    
                    gz4d::ogr::Polygon polygon = feature.GetGeometry();
                    if(polygon)
                    {
                        gz4d::ogr::LinearRing ring = polygon.getExteriorRing();
                        if(ring)
                            if(ring.isClockwise())
                                rings.push_back(std::vector<gz4d::Point<double> >(ring.rbegin(),ring.rend()));
                            else
                                rings.push_back(std::vector<gz4d::Point<double> >(ring.begin(),ring.end()));

                        for(unsigned int i=0; i < polygon.getNumInteriorRings(); ++i)
                        {
                            ring = polygon.getInteriorRing(i);
                            if(ring)
                                if(ring.isClockwise())
                                    rings.push_back(std::vector<gz4d::Point<double> >(ring.rbegin(),ring.rend()));
                                else
                                    rings.push_back(std::vector<gz4d::Point<double> >(ring.begin(),ring.end()));
                        }
                    }

                }
            }
            bbox = envelope;
            _scale = 2.0/bbox.getMaxLength();
            _offset_x = -bbox.getCenter()[0];
            _offset_y = -bbox.getCenter()[1];
            
            std::cerr << bbox.getMin() << " - " << bbox.getMax() << std::endl;
        }
        void Draw()
        {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            glColor3f(1.0,1.0,1.0);
            DrawArrays(points,GL_POINTS);
            for(std::vector<std::vector<gz4d::Point<double> > >::iterator r = rings.begin(); r!= rings.end(); ++r)
                DrawArrays(*r,GL_LINE_STRIP);
            SwapBuffers();
        }
};


int main(int argc, char * argv[])
{
    MyScene s(argv[1]);
    s("show_ogr");
    return 0;
}
