#include "engine/Window3D.h"
#include"S57_buoy.h"
#include"S57.h"
#include<gz4d/glo.h>
#include<GL/glut.h>
#include "engine/PosableStateBase.h"
#include "objects/GeoData/Glyph.h"
#include "objects/GeoData/Cone.h"
#include "objects/GeoData/Cube.h"
#include "objects/GeoData/Cylinder.h"
#include "objects/GeoData/SphereGlyph.h"
#include "objects/DynamicTides.h"
#include "engine/gzCommandInterface.h"
#include "utilities/DottedAddress.h"
#include "engine/Scene3D.h"
#include "MainWindow.h"
#include "engine/ViewManager.h"
#include "engine/PosableStateBase.h"
#include <map>

REGISTER_OBJECT(S57_buoy,"");

enum {SHP_DEFAULT, SHP_CONICAL, SHP_CAN, SHP_SPHERICAL, SHP_PILLAR, SHP_SPAR, SHP_BARREL, SHP_SUPER, SHP_ICE};
static Cube cube(1.0f); //SHP_DEFAULT
static Cone cone(1.3f, 1.0f, GL_FALSE); //SHP_CONICAL
static Cylinder cylinder(1.0f); //SHP_CAN
static SphereGlyph sphere(1.0f); //SHP_SPHERICAL
static Cube pillar(1.0f, 2.0f); //SHP_PILLAR
static Cylinder spar(1.0f, 3.0f); //SHP_SPAR
static Cylinder barrel(1.0f, 2.0f, 90.0f); //SHP_BARREL
static Cylinder superBuoy(3.0f, 1.5f); //SHP_SUPER
static Cone iceBuoy(1.0f, 2.0f, GL_FALSE); //SHP_ICE
Glyph *S57_buoy::buoyGlyph[] = {&cube, &cone, &cylinder, &sphere,
                                  &pillar, &spar, &barrel,
                                  &superBuoy, &iceBuoy};

static const char buoyShapeName[][32] = {"Unknown",
                                "Conical (nun, ogival)",
                                "Can (cylindrical)",
                                "Spherical",
                                "Pillar",
                                "Spar (spindle)",
                                "Barrel (tun)",
                                "Super-buoy (very large)",
                                "Ice buoy (shuttle-shaped)"
                               };

#include <iostream>

/*---------------------------------------------------------------------------*/
S57_buoy::S57_buoy()
{
    pointSize = 3;
    shape = SHP_DEFAULT;
}

/*---------------------------------------------------------------------------*/
S57_buoy::~S57_buoy()
{
}

/*---------------------------------------------------------------------------*/
void S57_buoy::DrawLowRes(float deg,Window3D * win)
{
    float scale, pitch;
    double x, y;
    DynamicTides *dTides;
    GLboolean draw2D = GL_FALSE;
    int renderMode = Glyph::Render3D;

    if (win)
    {
        scale = win->getViewInfo()->getPosable()->getScale();
        if (scale <= scamin) //use = because we already nudged scamax & scamin for roundoff
            return;
        scale = 0.0025f / scale; //make the buoys 5mm across if they would be any smaller (spheres get drawn as 2m wide)
        if (scale > 100) //just let the point be drawn.
        {
            S57_object::DrawLowRes(deg,win);
            return;
        }
    }
    else
        scale = 1.0;

    glColor3f(S57::dimFactor()*colour[0],S57::dimFactor()*colour[1],S57::dimFactor()*colour[2]);

    //Draw the buoy
    glPushMatrix();
    gz4d::Point<double> pos = pose->getPosition();
    gz4d::Point<double> wpos = win->getViewInfo()->getPosable()->getPosition();
    x = pos[X] - wpos[X];
    y = pos[Y] - wpos[Y];

    //first, account for tidal change in Z
    dTides = gzCommandInterface::getDefaultTides();
    if (dTides)
        glTranslatef(0.0, 0.0, dTides->getTideOffset(pos[X], pos[Y]) + 1.0);
    if (scale > 1.0)
    {
        if (scale > 20.0)
            scale = 20.0 + sqrt(scale - 20.0);
        glScalef(scale, scale, scale);
    }

    //draw the actual shape
    pitch = win->getViewInfo()->getPosable()->getOrientationAxis()[Pitch];
    if ((pitch > 75.0 && scale > 1.0) || (x * x + y * y > 2500)) //are we looking down or is the buoy more than 50 meters away from the center of workspace?
        draw2D = GL_TRUE;

    if (draw2D)
    {
        glDisable(GL_LIGHTING);
        if (pitch < 25.0 && !((MainWindow::getInterfaceType() == GeoCoastPilot) && renderingForPicking)) //only render in 3D depth for views near enough to a bridge view
            renderMode = Glyph::Render2D_inDepth;
        else
            renderMode = Glyph::Render2D_atScreen;
        buoyGlyph[shape]->draw(deg, renderMode); //draw 2D in front of all 3D stuff
        glEnable(GL_LIGHTING);
    }
    else
    {
        glEnable(GL_LIGHTING);
        buoyGlyph[shape]->draw(deg, renderMode);
    }

    //draw a line to the surface (assume at 0 for now, add an extra 10 meters...)
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
        glVertex3f(0.0, 0.0, 0.0);
        if (pos[Z] < 0)
            glVertex3f(0.0, 0.0, pos[Z] - 10.0);
        else
            glVertex3f(0.0, 0.0, -pos[Z] - 10.0);
    glEnd();

    //Draw labels near cursor
    if (selectedFlag || win->getCursorPosOnPlane(x, y, pos[Z])) //see if the cursor is nearby.  If so, show label.
    {
        x -= pos[X];
        y -= pos[Y];
        if (selectedFlag || ((x * x + y * y) < (900.0 * scale * scale))) // are we within 30m of the buoy?
        {
            glColor4f(S57::dimFactor(), S57::dimFactor(), S57::dimFactor(), 0.7f);
            //glLoadIdentity();
            if (renderMode == Glyph::Render2D_atScreen)
                win->printOver(0.0, 0.0, pos[Z] + 3.0, GetLabel());
            else
                win->printAt(0.0, 0.0, pos[Z] + 3.0, GetLabel());
        }
    }

    glPopMatrix();
}

/*---------------------------------------------------------------------------*/
void S57_buoy::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{
    S57_beacon::SetAttributes(item, info, objectNumber);

    // buoy shape
    if(item["BOYSHP"])
        shape = item["BOYSHP"].GetValue<int>();
    if(item["CATSPM"])
        categoryList = item["CATSPM"].GetValue<std::vector<int> >();
}

/*---------------------------------------------------------------------------*/
void S57_buoy::ShowS57ShapeInfo(Scene3D *scene)
{
	char infoLine[512];

    if (generateSpecialPurposeString(infoLine))
        scene->appendInfo(infoLine);
    if (shape > 0 && shape <= SHP_ICE)
    {
        sprintf(infoLine, "Shape:\t%s\n", buoyShapeName[shape]);
        scene->appendInfo(infoLine);
    }
}

