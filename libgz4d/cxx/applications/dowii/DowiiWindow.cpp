#include "DowiiWindow.h"

#include <gz4d/Rotation.h>
#include <math.h>
#include <iostream>

DowiiWindow::DowiiWindow(std::string t, boost::filesystem::path fn, gz4d::Dowii *d)
    : GlutWindow(t)
{
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    GLfloat p[4] = {1.0,1.0,1.0,0.0};
    glLightfv(GL_LIGHT0,GL_POSITION,p);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    x3d.Load(fn);
    dowii = d;
    dowii->scan();
    screen.setTransform(&transform);
}

void DowiiWindow::OnIdle()
{
    dowii->doIdle();
    transform.setScale(dowii->scale);
    transform.setRotation(gz4d::Quaternion(-dowii->heading,0.0,1.0,0.0)*gz4d::Quaternion(-dowii->roll,0.0,0.0,1.0)*gz4d::Quaternion(-dowii->pitch,1.0,0.0,0.0));//*gz4d::Quaternion(-1.57,1.0,0.0,0.0)
    postRedisplay();
}

void DowiiWindow::OnDisplay()
{
    screen.applyView(view);
    screen.applyTransform();
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    x3d.Draw();

    swapBuffers();
}

void DowiiWindow::OnReshape(int w, int h)
{
    screen.SetViewport(gz4d::Viewport(w,h));
}

void DowiiWindow::OnMouse(int button, int state, int x, int y)
{
}

void DowiiWindow::OnMotion(int x, int y)
{
}
