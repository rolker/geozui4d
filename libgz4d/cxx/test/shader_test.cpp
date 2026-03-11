// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/glo/Shader.h>
#include <gz4d/glo/Program.h>

#include<gz4d/glut/Application.h>
#include <gz4d/glo/Uniform.h>
#include <gz4d/time.h>
#include <gz4d/Point.h>

std::string fragSrc = "\
uniform float scale;\n\
void main()\n\
{\n\
  vec4 color = gl_Color;\n\
  vec4 coord = gl_FragCoord;\n\
  float i = sin(scale*sqrt(coord.x*coord.x+coord.y*coord.y));\n\
  gl_FragColor = color*i;\
}\n\
";


class MyScene: public gz4d::glut::Application<MyScene,gz4d::glut::Ortho<> >
{
    gz4d::glo::Shader<GL_FRAGMENT_SHADER> fragShader;
    gz4d::glo::Shader<GL_VERTEX_SHADER> vertShader;
    gz4d::glo::Program prog;
    gz4d::glo::Uniform scale;
    
    bool init;
public:
    MyScene():fragShader(context),vertShader(context),prog(context),init(false)
    {
    }


    void Idle()
    {
        if(scale)
        {
            gz4d::Time t = gz4d::time::Now();
            scale(1.0/(5.0+sin(0.5*t.secondsSinceEpoch().asSeconds())));
        }
        PostRedisplay();
    }
    
    void Draw()
    {
        glClearColor(0.5f, 0.0f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
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
            scale = prog.GetUniform("scale");
        }
        
        glDisable(GL_LIGHTING);
        
        
        glColor3f(1.0,1.0,1.0);
        
        glBegin(GL_QUADS);
        glVertex2f(-.5,.5);
        glVertex2f(-.5,-0.5);
        glVertex2f(.5,-.5);
        glVertex2f(.5,.5);
        glEnd();

        
        SwapBuffers();
    }
};

int main(int argc, char * argv[])
{
    MyScene s;
    s("Shaders");
    return 0;
}



