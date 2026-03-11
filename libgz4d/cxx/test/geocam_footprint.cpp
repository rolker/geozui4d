// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/glut/Application.h>
#include <gz4d/geocam/Platform.h>
#include <gz4d/data/Raster.h>
#include <gz4d/render/DistortionGrid.h>
#include <gz4d/glo/Color.h>

#include <gz4d/glo/Shader.h>
#include <gz4d/glo/Program.h>

std::string fragSrc = "\
void main()\n\
{\n\
vec4 color = gl_Color;\n\
float t = 0.5+gl_TexCoord[0].t/2.0;\n\
gl_FragColor = color*t;\
}\n\
";


class Footprint: public gz4d::glut::Application<Footprint, gz4d::glut::Ortho<> >
{
    typedef gz4d::Raster<gz4d::Vector<double,2> > GridType;
    typedef std::map<std::string,GridType> GridMap;
    GridMap _grids;
    
    typedef std::map<std::string,gz4d::DistortionGrid> DistGridMap;
    DistGridMap _dist_grids;
    
    gz4d::glo::Shader<GL_FRAGMENT_SHADER> fragShader;
    gz4d::glo::Program prog;
    
    bool init;
public:
    Footprint(const std::string &config):fragShader(context),prog(context),init(false)
    {
        setMinScale(0.0000001,false);
        
        using namespace gz4d::geocam;
        
        Platform::Ptr geocam = Platform::create(config);
        
        for(Platform::CameraMap::const_iterator c = geocam->getCameras().begin(); c != geocam->getCameras().end(); ++c)
        {
            std::cout << c->second->getInfo()  << std::endl;
            _grids[c->first].Resize(c->second->getImageWidth(),c->second->getImageHeight());
            gz4d::Point< double > offset = c->second->getOffset();
            gz4d::Vector< double, 2 >* last_row = nullptr;
            for(std::size_t row = 0; row < _grids[c->first].GetRows(); ++row)
            {
                //std::cout << row << " of " << _grids[c->first].GetRows() << "\t(" << c->first << ")" << std::endl;
                gz4d::Vector< double, 2 >* grid_row = _grids[c->first].GetRow(row);
                for(std::size_t col = 0; col < _grids[c->first].GetColumns(); ++col)
                {
                    gz4d::Point< double > v = c->second->imageToVector(col,row);
                    double scale = -offset[2]/v[2];
                    if(scale > 0.0)
                    {
                        gz4d::Point<double> p = offset+v*scale;
                        grid_row[col][0] = p[0];
                        grid_row[col][1] = p[1];
                    }
//                      else if(row > 0)
//                          grid_row[col] = last_row[col];
                }
                last_row = grid_row;
            }
          
        }
        
    }

    void drawBox(float size)
    {
        float hs = size/2.0f;
        glBegin(GL_LINE_LOOP);
        glVertex2d(-hs,-hs);
        glVertex2d(hs,-hs);
        glVertex2d(hs,hs);
        glVertex2d(-hs,hs);
        glEnd();
    }
    
    void Draw()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        glColor3f(1.0,1.0,1.0);

        drawBox(1.0);
        drawBox(100.0);
        drawBox(1000.0);
        
        glColor3f(1.0,0.25,0.25);
        drawBox(10000.0);
        
        if(!init)
        {
            fragShader.Source(fragSrc);
            fragShader.Compile();
            std::cerr << "compile status: " << fragShader.CompileStatus() << std::endl;
            std::cerr << "info log:\n" << fragShader.GetInfoLog() << std::endl;
            std::cerr << "src:\n" << fragShader.GetSource() << std::endl;
            prog.AttachShader(fragShader);
            prog.Link();
            std::cerr << "link status: " << prog.LinkStatus() << std::endl;
            std::cerr << "info log:\n" << prog.GetInfoLog() << std::endl;
            std::cerr << "validate: " << prog.Validate() << std::endl;
            std::cerr << "info log:\n" << prog.GetInfoLog() << std::endl;
            std::cerr << "active attributes count: " << prog.ActiveAttributesCount() << std::endl;
            std::cerr << "active uniforms count: " << prog.ActiveUniformsCount() << std::endl;
            std::cerr << prog.GetActiveUniform(0).name << std::endl;
            prog.Use();
            init = true;
        }
        else
            prog.Use();
        
        
        glPointSize(1.0);
        int c = 0;
        for(GridMap::const_iterator g = _grids.begin(); g != _grids.end(); ++g)
        {
            gz4d::Color color = gz4d::Black();
            color[c++] = 1.0;
            c %= 3;
            gz4d::glo::Color(color);
            _dist_grids[g->first].Draw(g->second,GL_POINTS);
        }
        
        prog.FixedFunctionality();

        glPushMatrix();
        glColor3f(0.5,0.5,0.5);
        glTranslatef(-_offset_x,-_offset_y,0.0);
        drawBox(1.0);
        drawBox(10.0);
        glPopMatrix();
        
        
        SwapBuffers();
    }
};


int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        std::cerr << "geocam_footprint config.xml" << std::endl;
        exit(1);
    }

    Footprint fp(argv[1]);
    fp("geoCamera footprint");
    
    return 0;
}
