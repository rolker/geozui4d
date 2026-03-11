// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/OffScreenContext.h>
#include <gz4d/Image.h>
#include <gz4d/image/io.h>
#include <GL/glu.h>

#include <iostream>

class TestOSMesa
{
public:
    TestOSMesa()
    {
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glShadeModel (GL_SMOOTH);

        glEnable(GL_DEPTH_TEST);
    }

    void OnReshape(int w, int h)
    {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective( /* field of view in degree */ 90.0,
                        /* aspect ratio */ (float)w/(float)h,
                        /* Z near */ 1.0,
                        /* Z far */ 1000.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(50.0, 0.0, 75.0,
                  50.0, 0.0, 0.0,
                  0.0, 1.0, 0.0);
    }

    void OnDisplay()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawBuffer(GL_FRONT);
        glColor4f(1.0,0.0,0.0,1.0);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0,0.0);
        glVertex2f(-100.0,-100.0);
        glVertex2f(100.0,-100.0);
        glEnd();
        glFlush();
    }
};

int main(int argc, char * argv[])
{
    std::cerr << "main\n";
    gz4d::OffScreenContext osc;
    osc.GetRaster()->Resize(1280,1024);
    osc.MakeCurrent();
    TestOSMesa tosm;
    tosm.OnReshape(osc.GetRaster()->GetColumns(),osc.GetRaster()->GetRows());
    tosm.OnDisplay();
    glFinish();
    gz4d::ImageRGB8 i(osc.GetRaster());
    i.Save("osmesa_test.tiff");
}
