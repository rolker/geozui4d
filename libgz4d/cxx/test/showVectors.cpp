#include<gz4d/glut/Application.h>
#include <gz4d/Point.h>
#include <vector>
#include <fstream>
#include <gz4d/render/DrawArrays.h>
#include <gz4d/glo/Normal.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/DisplayList.h>
#include <gz4d/vector/operations.h>
#include <gz4d/Rotation.h>

class MyScene: public gz4d::glut::Application<MyScene>
{
    struct Ping
    {
        gz4d::Point<float> position;
        float sog;
        gz4d::Angle<float,gz4d::Degree> cmg;
        gz4d::Angle<float,gz4d::Degree> heading;
        float bottom[4];
        std::vector<gz4d::Point<float> > currents;
        std::vector<float> radialVelocities[4];
        Ping const &operator=(Ping const &o){memcpy(this,&o,sizeof(Ping)); return *this;}
    };

    std::vector<Ping> data;

    gz4d::Point<float> z0;
    gz4d::Point<float> dz;
    int binCount;

    gz4d::glo::DisplayList displayList;

    public:
        
    void Draw()
    {
        if(!displayList.Call())
        {
            glRotatef(-90.0,1,0,0);
            glColor3f(1.0,1.0,1.0);

            glDisable(GL_LIGHTING);
            glEnable(GL_COLOR_MATERIAL);
            glShadeModel (GL_SMOOTH);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_NORMALIZE);

            float zfactor = cos(gz4d::Radians(30.0));
            float xyfactor = sin(gz4d::Radians(30.0));

            int n = 0;
            int bin_size = 5;
            std::vector<gz4d::Point<float> > bins(binCount/bin_size,gz4d::Point<float>());
            std::vector<int> counts(binCount/bin_size,0);
            gz4d::Point<float> avg_position(0.0,0.0,0.0);

            glBegin(GL_LINES);
            for(int i = 0; i < data.size(); ++i)
            {
                if(i>0)
                {
/*                    glColor3f(0.0,0.0,0.0);
                    gz4d::glo::Vertex(data[i-1].position);
                    glColor3f(0.3,0.3,0.8);
                    gz4d::glo::Vertex(data[i].position);*/
                }

                bool bottom_ok = true;
                for(int j = 0; j < 4; ++j)
                    if(gz4d::IsNan(data[i].bottom[j]))
                       bottom_ok = false;

                if(bottom_ok)
                {
                    gz4d::Point<float> bottom_track((-data[i].bottom[0]+data[i].bottom[1])/(2.0*xyfactor),(data[i].bottom[2]-data[i].bottom[3])/(2.0*xyfactor),(data[i].bottom[0]+data[i].bottom[1]+data[i].bottom[2]+data[i].bottom[3])/(4.0*zfactor));

                    gz4d::Rotation<float> h(gz4d::Radians(data[i].heading),gz4d::Point<float>(0.0,0.0,-1.0));

                    bottom_track = h(bottom_track);

                    glColor3f(1.0,0.0,0.0);
                    gz4d::glo::Vertex(data[i-1].position);
                    gz4d::glo::Vertex(data[i-1].position-bottom_track);
                }

                gz4d::Point<float> base = data[i].position + z0;

                avg_position += data[i].position;
                int m = 0;
                int k = 0;
                ++n;
                for(int j = 0; j < data[i].currents.size(); ++j)
                {
                    if(!gz4d::IsNan(data[i].currents[j][0]) && !gz4d::IsNan(data[i].currents[j][1]) && !gz4d::IsNan(data[i].currents[j][2]))
                    {
                        bins[m] += data[i].currents[j];
                        ++counts[m];
/*                        glColor3f(0.0,0.0,0.0);
                        gz4d::glo::Vertex(base + dz*j);
                        glColor3f(1.0,1.0,1.0);
                        gz4d::glo::Vertex(base + dz*j + data[i].currents[j]*15.0);*/
                    }
                    ++k;
                    if(k == bin_size)
                    {
                        if(n == bin_size)
                        {
                            if(counts[m] > 0)
                            {
                                glColor3f(0.0,0.0,0.0);
                                gz4d::glo::Vertex(avg_position/float(bin_size)+ z0 + dz*(j-bin_size/2.0));
                                glColor3f(1.0,1.0,1.0);
                                gz4d::glo::Vertex(avg_position/float(bin_size)+ z0 + dz*(j-bin_size/2.0) + bins[m]*50.0/float(counts[m]));
                            }
                        }
                        k = 0;
                        ++m;
                        if(n == bin_size && m == bin_size)
                        {
                            n = 0;
                            bins = std::vector<gz4d::Point<float> >(binCount/bin_size,gz4d::Point<float>());
                            counts = std::vector<int>(binCount/bin_size,0);
                            avg_position = gz4d::Point<float>(0.0,0.0,0.0);
                        }
                    }
                }
            }
            glEnd();

            glBegin(GL_TRIANGLES);
            glColor3f(0.0,1.0,0.0);
            glNormal3f(0.0,0.0,1.0);

            for(int i = 0; i < data.size(); ++i)
            {
                gz4d::Rotation<float> cmg(gz4d::Radians(data[i].cmg),gz4d::Point<float>(0.0,0.0,-1.0));
                
                gz4d::Point<float> ship_velocity = cmg(gz4d::Point<float>(0.0,data[i].sog,0.0));
                
                gz4d::Point<float> ship_velocity_side = cmg(gz4d::Point<float>(data[i].sog*0.1,0.0,0.0));
                
                gz4d::glo::Vertex(data[i].position+ship_velocity);
                gz4d::glo::Vertex(data[i].position-ship_velocity_side);
                gz4d::glo::Vertex(data[i].position+ship_velocity_side);
            }
            glEnd();


            displayList.End();
        }
    }

        MyScene(std::string fname)
        {
            std::ifstream in(fname.c_str());
            std::string tmp;
            in >> tmp >> tmp >> tmp;
            in >> tmp >> tmp;
            in >> tmp >> tmp;
            in >> tmp >> z0[2];
            z0[2] = -z0[2];
            in >> tmp >> dz[2];
            dz[2] = -dz[2];
            in >> tmp >> binCount;

            std::cout << z0[2] << ", " << dz[2] << ", " << binCount << std::endl;
            in >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp;
            in >> tmp >> tmp >> tmp >> tmp;
            for(int i = 0; i < binCount; ++i)
                in >> tmp >> tmp >> tmp >> tmp;

            for(int i = 0; i < 4; ++ i)
                for(int j = 0; j < binCount; ++j)
                    in >> tmp;

            while(!in.eof())
            {
                Ping p;

                float x,y;
                in >> tmp >> x >> y;

                p.position = gz4d::Point<float>(x,y,0.0);

                float cmg;
                in >> p.sog >> cmg;
                p.cmg = gz4d::Angle<float,gz4d::Degree>(cmg);

                float heading;
                in >> heading;
                p.heading = gz4d::Angle<float,gz4d::Degree>(heading);

                std::string bt[4];
                for(int i = 0; i < 4; ++ i)
                {
                    in >> bt[i];
                    p.bottom[i] = atof(bt[i].c_str());
                }

                for(int i = 0; i < binCount; ++i)
                {
                    std::string us,vs,ws,es;
                    in >> us >> vs >> ws >> es;
                    float u,v,w,e;
                    u = atof(us.c_str());
                    v = atof(vs.c_str());
                    w = atof(ws.c_str());
                    e = atof(es.c_str());
                    p.currents.push_back(gz4d::Point<float>(u,v,w));
                }

                for(int i = 0; i < 4; ++i)
                    for(int j = 0 ; j < binCount; ++j)
                    {
                        std::string rvs;
                        in >> rvs;
                        float rv = atof(rvs.c_str());
                        p.radialVelocities[i].push_back(rv);
                    }
                data.push_back(p);
            }
            std::cout << data.size() << std::endl;
        }

};

int main(int argc, char *argv[])
{
    MyScene s(argv[1]);
    s("vectors");
}
