#include<gz4d/glo.h>
#include<gz4d/glut/Application.h>
#include<gz4d/glo/VertexArray.h>
#include<gz4d/glo/BufferObject.h>
#include<gz4d/glo/FramebufferObject.h>
#include<gz4d/glo/Texture.h>
#include <iostream>
#include <gz4d/Event.h>
#include <vector>

class MyScene: public gz4d::glut::Application<MyScene>
{
    gz4d::glo::Buffer abo;
    gz4d::glo::Buffer eabo;
    gz4d::glo::Framebuffer fbo;
    gz4d::glo::Renderbuffer rb;
    gz4d::glo::Texture texture;
    std::vector<GLsizei> counts;
    std::vector<GLvoid const*> indexOffsets;

    gz4d::Time lastTime;

    public:
        MyScene()
        :abo(context,GL_ARRAY_BUFFER),eabo(context,GL_ELEMENT_ARRAY_BUFFER),fbo(context), rb(context,GL_DEPTH_COMPONENT),texture(context,GL_TEXTURE_2D,GL_RGB8),lastTime(gz4d::time::Now())
        {
        }

    void Draw()
    {

        if(counts.empty())
        {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-.5,.5,-.5,.5,1.0,5.0);
            glTranslatef(0.0,0.0,-2.5);
            glMatrixMode(GL_MODELVIEW);
            glDrawBuffer(GL_FRONT);
            
            std::vector<gz4d::glo::VertexArray::T2F_V3F> v;
            v.resize(400);
            for (int i = 0; i < 20; ++i)
                for(int j = 0; j < 20; ++j)
                {
                    v[i*20+j].x = -.75 + j*.075;
                    v[i*20+j].y = -.75 + i*.075+.1*sin(j*.5);
                    v[i*20+j].z = 0.0;
                    v[i*20+j].ts = j/20.0;
                    v[i*20+j].tt = i/20.0;
                }
                abo.Bind();
            abo.Data(v.size()*sizeof(gz4d::glo::VertexArray::T2F_V3F),&v.front());
            abo.Unbind();
            
            std::vector<uint16_t> indecies;
            for (int i = 0; i < 19; ++i)
            {
                for(int j = 0; j < 20; ++j)
                {
                    indecies.push_back(i*20+j);
                    indecies.push_back((i+1)*20+j);
                }
                counts.push_back(40);
                indexOffsets.push_back((const GLvoid*)(i*40*sizeof(uint16_t)));
            }
            eabo.Bind();
            eabo.Data(indecies.size()*sizeof(uint16_t),&indecies.front());
            eabo.Unbind();
            
            fbo.Bind();
            
            texture.Bind();
            texture.Image(512,512);
            texture.MinFilter(GL_NEAREST);
            texture.MagFilter(GL_NEAREST);
            fbo.Attach(texture);
            
            
            rb.Bind();
            rb.Storage(512,512);
            fbo.Attach(rb);
            
            fbo.CheckStatus();
            
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-.5,.5,-.5,.5,1.0,5.0);
            glTranslatef(0.0,0.0,-2.5);
            glMatrixMode(GL_MODELVIEW);
            
            glClearColor(0.0,0.0,0.0,1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            fbo.Unbind();
            texture.Unbind();
            rb.Unbind();

        }

        gz4d::Time now = gz4d::time::Now();
        gz4d::time::Duration tRemaining =  lastTime + gz4d::time::Duration(1.0/45.0) - now;
        if(tRemaining > gz4d::time::Duration(0.0))
            Sleep(tRemaining);

        lastTime = gz4d::time::Now();

        fbo.Bind();

        glPushAttrib(GL_VIEWPORT_BIT| GL_COLOR_BUFFER_BIT);
        glViewport(0,0,512, 512);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);


        glClear(GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glColor4f(0.0,0.0,0.0,0.05);
        glRectf(-2.0,2.0,2.0,-2.0);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glColor4f(.3,.4,1.0,1.0);
        glPushMatrix();
        glRotatef(fmod(gz4d::time::Now().decompose().second,10.0)*36.0,0.0,1.0,0.0);
        Teapot(0.65);
        glPopMatrix();
        glPopAttrib();
        fbo.Unbind();

        glDrawBuffer(GL_BACK);

        glClearColor(0.0,0.2,0.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        abo.Bind();
        eabo.Bind();
        texture.Bind();
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glEnable(GL_TEXTURE_2D);
        glInterleavedArrays(GL_T2F_V3F,0,NULL);
        glDisable(GL_BLEND);
        glColor3f(1.0,1.0,1.0);
        glMultiDrawElements(GL_QUAD_STRIP,&counts.front(),GL_UNSIGNED_SHORT,&indexOffsets.front(),counts.size());

        glDisable(GL_TEXTURE_2D);
        texture.Unbind();
        glDisableClientState(GL_VERTEX_ARRAY);
        abo.Unbind();
        eabo.Unbind();

        PostRedisplay();
    }

    void Idle()
    {
        PostRedisplay();
    }

};

int main()
{
    try
    {
        MyScene s;
        s("vbo");
    }
    catch(gz4d::Exception e)
    {
        std::cerr << "Oops! " << e.what() << std::endl;
        return 1;
    }
}
