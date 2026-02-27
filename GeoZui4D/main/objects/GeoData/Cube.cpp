/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "Cube.h"
#include "engine/ToglWindow.h"

#ifdef EXPERIMENT_GLYPHS
#define MULT 4
#define CUBE_SIZE	float(0.5)
#else
#define MULT 1
#define CUBE_SIZE	float(1.3)
#endif

//2D representation
static unsigned char outline[32] = {0x00, 0x00,
                                    0x7F, 0xFF,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x80, 0x01,
                                    0x7F, 0xFF};

static unsigned char filling[32] = {0x00, 0x00,
                                    0x00, 0x00,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x7F, 0xFE,
                                    0x00, 0x00};

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

Cube::Cube(float width, float hgt)
{
    if(width == 0.0)
        init(CUBE_SIZE,CUBE_SIZE);
    else if(hgt == 0.0)
        init(width,width);
    else
        init(width,hgt/2.0);

    displayList = 0;
}

void Cube::init(float width, float height)
{
        // run around the sides 
        Points[0][0] = -width;
        Points[0][1] = -width;
        Points[0][2] = height;
        Points[1][0] = -width;
        Points[1][1] = -width;
        Points[1][2] = -height;

        Points[2][0] = -width;
        Points[2][1] = width;
        Points[2][2] = height;
        Points[3][0] = -width;
        Points[3][1] = width;
        Points[3][2] = -height;

        Points[4][0] = width;
        Points[4][1] = width;
        Points[4][2] = height;
        Points[5][0] = width;
        Points[5][1] = width;
        Points[5][2] = -height;

        Points[6][0] = width;
        Points[6][1] = -width;
        Points[6][2] = height;
        Points[7][0] = width;
        Points[7][1] = -width;
        Points[7][2] = -height;
}

/*--------------------------------------------------------------------------*/
void Cube::draw(float deg, int renderMode)
{
        if (renderMode != Render3D) //using2D
        {
                float x = 0.0, y = 0.0, z = 0.0;

                if (renderMode == Render2D_atScreen)
                {
                        if (!ToglWindow::beginRaster2D(x, y, z)) //not at a valid screen location
                                return;
                }
                else
                        glRasterPos3f(x, y, z);

                glBitmap(16, 16, 8, 8, 0, 0, filling);
                glPushAttrib(GL_CURRENT_BIT);
                glColor4f(0.0, 0.0, 0.0, 1.0);

                glRasterPos3f(x, y, z);
                glBitmap(16, 16, 8, 8, 0, 0, outline);
                glPopAttrib();

                if (renderMode == Render2D_atScreen)
                        ToglWindow::endRaster2D();
                return;
        }

        if(!displayList)
        {
            //displayList = glGenLists(1);
            //glNewList(displayList,GL_COMPILE);

            glShadeModel(GL_FLAT);
    #ifdef EXPERIMENT_GLYPHS
            glRotatef(45, 1.0, -1.0, 0.2f);
    #endif
    
            //draw the sides
            glBegin(GL_QUAD_STRIP);
            glNormal3f(-1.0, 0.0, 0.0);
            glVertex3fv(Points[0]); 
            glVertex3fv(Points[1]); 
            glVertex3fv(Points[2]); 
            glVertex3fv(Points[3]); 
            glNormal3f(0.0, 1.0, 0.0);
            glVertex3fv(Points[4]); 
            glVertex3fv(Points[5]); 
            glNormal3f(1.0, 0.0, 0.0);
            glVertex3fv(Points[6]); 
            glVertex3fv(Points[7]);		
            glNormal3f(0.0, -1.0, 0.0);
            glVertex3fv(Points[0]); 
            glVertex3fv(Points[1]);
            glEnd();
    
            glBegin(GL_QUADS);
            glNormal3f(0.0, 0.0, 1.0);
            glVertex3fv(Points[0]); 
            glVertex3fv(Points[2]); 
            glVertex3fv(Points[4]); 
            glVertex3fv(Points[6]); 
            glEnd();
            //glEndList();
        }

        //glCallList(displayList);
}









