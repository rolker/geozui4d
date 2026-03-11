// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/glut/Application.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdio>

class ViewAudio: public gz4d::glut::Application<ViewAudio>
{
    private:
        float vScale;
        float hScale;
        float tx,ty;
        int channels;
        int samples;
        std::vector<float> data;
        int startVSY,startTX,startTY;
        float iVScale;
        bool vScaling,translating;
        int width, height;
        GLuint waveList, chunkGridList, grid10000List, grid1000List, grid100List, grid10List;

    public:
        ViewAudio(std::string infile)
        {
            tx = ty = 0.0;
            vScale = 1.0;
            vScaling = false;
            translating = false;
            waveList = chunkGridList = grid10000List = grid1000List = grid100List = grid10List =0;

            FILE * in = fopen(infile.c_str(),"r");
            samples = 0;
            if(in)
            {
                channels = 1;
                while(!feof(in))
                {
                    int c = fgetc(in);
                    if(!samples && c == ',')
                        channels++;
                    if (c=='\n')
                        samples++;
                }
                rewind(in);
                data.resize(samples*channels);
                for(int i = 0; i<samples; i++)
                    for(int j = 0; j < channels; j++)
                    {
                        if(j < channels-1)
                            fscanf(in,"%f, ",&data[i*channels+j]);
                        else
                            fscanf(in,"%f\n",&data[i*channels+j]);
                    }
                fclose(in);
                hScale = 2/float(samples);
                tx = -samples/4;
            }
            else
            {
                channels = 0;
                hScale = 1.0;
            }
        }

        void OnIdle()
        {
            //postRedisplay();
        }

        void Draw()
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawBuffer(GL_BACK);
            glPushMatrix();

            glTranslatef(2*tx*hScale,ty,0.0);
            glScalef(hScale,vScale,1.0);

            float max = 1.0;

            if(!grid10000List)
            {
                grid10000List = glGenLists(1);
                glNewList(grid10000List,GL_COMPILE);
                glBegin(GL_LINES);
                glColor3f(.7,.7,.7);
                for(int i=0;i<samples+10000;i+=10000)
                {
                    glVertex2f(i,max);
                    glVertex2f(i,-max);
                }
                glEnd();
                glEndList();
            }

            if(!grid1000List)
            {
                grid1000List = glGenLists(1);
                glNewList(grid1000List,GL_COMPILE);
                glBegin(GL_LINES);
                glColor3f(.6,.6,.6);
                for(int i=0;i<samples+1000;i+=1000)
                {
                    glVertex2f(i,max);
                    glVertex2f(i,-max);
                }
                glEnd();
                glEndList();
            }
            
            if(!grid100List)
            {
                grid100List = glGenLists(1);
                glNewList(grid100List,GL_COMPILE);
                glBegin(GL_LINES);
                glColor3f(.5,.5,.5);
                for(int i=0;i<samples+100;i+=100)
                {
                    glVertex2f(i,max);
                    glVertex2f(i,-max);
                }
                glEnd();
                glEndList();
            }

            if(!grid10List)
            {
                grid10List = glGenLists(1);
                glNewList(grid10List,GL_COMPILE);
                glBegin(GL_LINES);
                glColor3f(.2,.2,.2);
                for(int i=0;i<samples+10;i+=10)
                {
                    glVertex2f(i,max);
                    glVertex2f(i,-max);
                }
                glEnd();
                glEndList();
            }
            
            if(!chunkGridList)
            {
                chunkGridList = glGenLists(1);
                glNewList(chunkGridList,GL_COMPILE);
                glBegin(GL_LINES);
                glColor3f(.9,.8,1.0);
                for(int i=0;i<samples+900;i+=900)
                {
                    glVertex2f(i,max);
                    glVertex2f(i,-max);
                }
                glEnd();
                glEndList();
            }
            

            
            if(!waveList)
            {
                waveList = glGenLists(1);
                glNewList(waveList,GL_COMPILE);
            
                //draw grids
    
                glBegin(GL_LINES);
    
                glColor3f(.8,.8,.8);
                glVertex2i(0,0);
                glVertex2i(samples,0);
    
                glColor3f(.5,.5,.8);
                glVertex2f(0,max);
                glVertex2f(samples,max);
                glVertex2f(0,-max);
                glVertex2f(samples,-max);
                glEnd();
    
    
                glPointSize(3.0);
                glBegin(GL_POINTS);
                for(int j = 0; j < channels; j++)
                {
                    switch(j)
                    {
                        case 0:
                            glColor3f(1.0,0.0,0.0);
                            break;
                        case 1:
                            glColor3f(0.0,1.0,0.0);
                            break;
                        default:
                            glColor3f(0.0,0.0,1.0);
                    }
                    for(int i = 0; i < samples; i++)
                        glVertex2f(i,data[i*channels+j]);
                }
                glEnd();

                glLineWidth(1.0);
                for(int j = 0; j < channels; j++)
                {
                    switch(j)
                    {
                        case 0:
                            glColor3f(1.0,0.0,0.0);
                            break;
                        case 1:
                            glColor3f(0.0,1.0,0.0);
                            break;
                        default:
                            glColor3f(0.0,0.0,1.0);
                    }
                    glBegin(GL_LINE_STRIP);
                    for(int i = 0; i < samples; i++)
                        glVertex2f(i,data[i*channels+j]);
                    glEnd();
                }
                glEndList();
            }

            float pixPerUnit = hScale*(width/2);
            if(pixPerUnit*10000>25)
                glCallList(grid10000List);
            if(pixPerUnit*1000>25)
                glCallList(grid1000List);
            if(pixPerUnit*100>25)
                glCallList(grid100List);
            if(pixPerUnit*10>25)
                glCallList(grid10List);

            if(pixPerUnit*900>25)
                glCallList(chunkGridList);

            glCallList(waveList);
            glPopMatrix();
            SwapBuffers();
        }

        void OnReshape(int w, int h)
        {
            width = w;
            height = h;
            glViewport(0, 0, w, h);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(-1,1,-1,1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        void OnMouse(int button, int state, int x, int y)
        {
            switch(button)
            {
                case GLUT_LEFT_BUTTON:
                    if(state == GLUT_DOWN)
                    {
                        startTX = x;
                        startTY = y;
                        translating = true;
                    }
                    else
                        translating = false;
                    break;
                case GLUT_RIGHT_BUTTON:
                    if(state == GLUT_DOWN)
                    {
                        startVSY = y;
                        iVScale = vScale;
                        vScaling = true;
                    }
                    else
                        vScaling = false;
                    break;
                case 3:
                    hScale *= 1.25;
                    PostRedisplay();
                    break;
                case 4:
                    hScale *= 0.8;
                    PostRedisplay();
                    break;
                default:
                    break;
            }

        }

        void OnMotion(int x, int y)
        {
            if(vScaling)
            {
                vScale = iVScale * exp((startVSY-y)/10.0);
                PostRedisplay();
            }
            if(translating)
            {
                tx += (x-startTX)/(hScale*width);
                ty -= 2*(y-startTY)/float(height);
                startTX = x;
                startTY = y;
                PostRedisplay();
            }
        }
};

int main(int argc, char * argv[])
{
    ViewAudio v(argv[1]);
    v("View Audio");
    return 0;
}
