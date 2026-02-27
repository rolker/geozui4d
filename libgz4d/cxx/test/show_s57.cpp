#include <gz4d/s57/S57.h>
#include <gz4d/glut/Application.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/render/DrawArrays.h>
#include <gz4d/render/DrawPolygon.h>
#include <boost/thread/thread.hpp>
#include <gz4d/render/Tessellation.h>
#include <gz4d/geo/WGS84.h>
#include <gz4d/ogr/link.h>

void tessVertex(double *v)
{
    glVertex3dv(v);
}

class S57Viewer: public gz4d::glut::Application<S57Viewer>
{
    int width,height;
    double dpp; // degrees per pixel
    gz4d::s57::S57 enc;

    gz4d::Vector<float,3> center;
    gz4d::Vector<float,3> viewAngles;
    gz4d::Box3f vbbox;

    struct CellInfo
    {
        gz4d::s57::Cell::Data::Ptr data;
        GLuint displayLists;
        int age;
        CellInfo():displayLists(0),age(1){}
    };

    std::map<gz4d::s57::Cell::Ptr,CellInfo> cell_map;
    bool loading;

    public:
        S57Viewer(std::string const& f)
    :width(1280),height(1024),dpp(0.0),enc(f),loading(false)
        {
            InitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
            InitWindowSize(1280,1024);
            center = enc.getBBox().getCenter();
            viewAngles = enc.getBBox().getSizes();
        }

        void Reshape(int w, int h)
        {
            width = w;
            height = h;
            glViewport(0, 0, w, h);

            double l,r,b,t;


            double aspect = double(w)/double(h);
            double vaspect = viewAngles[0]/viewAngles[1];

            if (vaspect > aspect)
            {
                l = center[0] - viewAngles[0]/2.0;
                r = center[0] + viewAngles[0]/2.0;
                b = center[1] - (viewAngles[0]/aspect)/2.0;
                t = center[1] + (viewAngles[0]/aspect)/2.0;
            }
            else
            {
                l = center[0] - viewAngles[1]*aspect/2.0;
                r = center[0] + viewAngles[1]*aspect/2.0;
                b = center[1] - viewAngles[1]/2.0;
                t = center[1] + viewAngles[1]/2.0;
            }

            dpp = (r - l)/double(width);
            //std::cerr << "dpp: " << dpp << std::endl;

            vbbox = gz4d::Box3f();
            vbbox.expand(gz4d::Vector<float,3>(l,b,0.0));
            vbbox.expand(gz4d::Vector<float,3>(r,t,0.0));

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(l,r,b,t,-1000.0,1000.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        void Draw()
        {
            for(std::map<gz4d::s57::Cell::Ptr,CellInfo>::iterator i = cell_map.begin(); i != cell_map.end(); ++i)
                i->second.age += 1;

            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glColor3f(1.0,1.0,1.0);

            int scale = dpp*10000000;
            //std::cerr << "scale: " << scale << std::endl;

            gz4d::s57::Cell::List cl;// = enc.intersects(vbbox,scale);

            std::multimap<int,gz4d::s57::Cell::Ptr> sortedCells;
            for (gz4d::s57::Cell::List::iterator c = cl.begin(); c != cl.end(); ++c)
                sortedCells.insert(sortedCells.end(),std::multimap<int,gz4d::s57::Cell::Ptr>::value_type((*c)->getScale(),*c));

            loading = false;
            bool builtList = false;

            for(std::multimap<int,gz4d::s57::Cell::Ptr>::iterator c = sortedCells.begin(); c != sortedCells.end(); ++c)
            {
                cell_map[c->second].age -= 1;

                if(!cell_map[c->second].displayLists)
                {
                    if(!cell_map[c->second].data)
                        cell_map[c->second].data = c->second->Load();

                    if(builtList) // only build lists for one cell per rendering loop
                        loading = true;
                    else
                    {
                        gz4d::s57::Cell::Data::Ptr data = cell_map[c->second].data;

                        if(data)
                        {
                            cell_map[c->second].displayLists = glGenLists(4);
                            if(cell_map[c->second].displayLists)
                            {
                                glNewList(cell_map[c->second].displayLists,GL_COMPILE);

                                glPointSize(5);
                                glColor3f(0.0,1.0,0.0);
                                gz4d::DrawArrays(data->buoys,GL_POINTS);
                                glColor3f(1.0,0.0,0.0);
                                gz4d::DrawArrays(data->beacons,GL_POINTS);

                                glColor3f(0.3,0.3,0.3);
                                glLineWidth(2.5);
                                for(std::vector<std::vector<gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF> > >::iterator l = data->roadway.begin(); l != data->roadway.end(); ++l)
                                    gz4d::DrawArrays(*l,GL_LINE_STRIP);


                                glColor3ub(209,221,239);
                                data->rivers.Draw();
                                data->lake_area.Draw();

                                glColor3ub(244,232,193);
                                data->land_area.Draw();

                                glColor3f(1.0,1.0,1.0);
                                data->dredged_area.Draw();

                                glEndList();
    
                                glNewList(cell_map[c->second].displayLists+1,GL_COMPILE);

                                for(gz4d::s57::Cell::Data::DepthAreaMap::const_iterator d = data->depth_areas.begin(); d != data->depth_areas.end(); ++d)
                                {
                                    if(d->first.first < 0.0)
                                        glColor3ub(214,219,201); // tidal
                                    else if (d->first.first < 18.1)
                                        glColor3ub(209,221,239); // blue 1
                                    else if (d->first.first < 91.4)
                                        glColor3ub(221,234,247); // blue 2
                                    else
                                        glColor3f(1.0,1.0,1.0);
                                    d->second.Draw();
                                }
                                glEndList();
    
                                glNewList(cell_map[c->second].displayLists+2,GL_COMPILE);
    
                                for(gz4d::s57::Cell::Data::DepthAreaMap::const_iterator d = data->depth_areas.begin(); d != data->depth_areas.end(); ++d)
                                { 
                                    if(d->first.first < 0.0)
                                        glColor3ub(214,219,201); // tidal
                                    else if (d->first.first < 5.4)
                                        glColor3ub(209,221,239); // blue 1
                                    else
                                        glColor3f(1.0,1.0,1.0);
                                    d->second.Draw();
                                }
                                glEndList();

                                glNewList(cell_map[c->second].displayLists+3,GL_COMPILE);
                                glColor3f(1.0,0.5,0.5);
                                data->dumping_ground.Draw();
                                glEndList();

                                cell_map[c->second].data.reset();
                                builtList = true;
                            }
                        }
                        else
                            loading = true;
                    }
                }
                if(cell_map[c->second].displayLists)
                {
                    glCallList(cell_map[c->second].displayLists);
                    if(scale > 100000)
                        glCallList(cell_map[c->second].displayLists+1);
                    else
                        glCallList(cell_map[c->second].displayLists+2);
                    glCallList(cell_map[c->second].displayLists+3);
                }


            }

            SwapBuffers();

            for(std::map<gz4d::s57::Cell::Ptr,CellInfo>::iterator i = cell_map.begin(); i != cell_map.end();)
            {
                if(i->second.age > 25 && i->first->getScale() < 1000000)
                {
                    glDeleteLists(i->second.displayLists,4);
                    std::map<gz4d::s57::Cell::Ptr,CellInfo>::iterator di = i;
                    ++i;
                    cell_map.erase(di);
                }
                else
                    ++i;
            }
        }

        void Mouse(int button, int state, int x, int y)
        {
            y = height - y;

            if(button == 3 && state == GLUT_DOWN) // zoom in
            {
                center[0] = vbbox.getMin()[0]+vbbox.getSizes()[0]*x/double(width);
                center[1] = vbbox.getMin()[1]+vbbox.getSizes()[1]*y/double(height);

                viewAngles = vbbox.getSizes() *= 0.9;

                center[0] -= viewAngles[0]*(x-width/2)/double(width);
                center[1] -= viewAngles[1]*(y-height/2)/double(height);

                Reshape(width,height);
                PostRedisplay();
            }
            if(button == 4 && state == GLUT_DOWN) // zoom out
            {
                viewAngles /= 0.9;
                Reshape(width,height);
                PostRedisplay();
            }
        }

        void Idle()
        {
            if(loading)
                PostRedisplay();
            else
                boost::thread::yield();
        }
};


int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        std::cerr << "usage: show_s57 catalog.xml" << std::endl;
        exit(1);
    }

    S57Viewer sv(argv[1]);
    sv("S57 Viewer");
    return 0;
}
