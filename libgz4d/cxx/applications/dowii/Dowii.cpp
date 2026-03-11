// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "Dowii.h"

#include <iostream>
#include <gz4d/time.h>
#include <GL/glut.h>
#include <math.h>

gz4d::Dowii::Dowii()
{
    heading = 0.0;
    scale = 4.0;
    pitch = 0.0;
    roll = 0.0;
    lastTime = gz4d::time::Now();
    for(int i = 0; i < SMOOTH_BUFFER; i++)
    {
        w[i][0] = 0;
        w[i][1] = 0;
        w[i][2] = 0;
    }
    wi = 0;
    zoomingIn = false;
    zoomingOut = false;
    yawing = false;
}

void gz4d::Dowii::OnButton(unsigned short id, unsigned short b, bool down)
{
    if (b == WIIMOTE_BUTTON_H)
        exit(0);
    if (b == WIIMOTE_BUTTON_P)
        if (down)
            zoomingIn = !zoomingOut;
        else
            zoomingIn = false;
    if (b == WIIMOTE_BUTTON_M)
        if (down)
            zoomingOut = !zoomingIn;
        else
            zoomingOut = false;
    if (b == WIIMOTE_BUTTON_B)
        if (down)
            yawing = true;
        else
            yawing = false;
}

void gz4d::Dowii::OnIdle()
{
    double now = gz4d::time::Now();

    double delta = now-lastTime;

    if(zoomingIn)
        scale *= pow(3,delta);
    if(zoomingOut)
        scale /= pow(3,delta);
    if (scale < 0.1)
        scale = 0.1;
    if (scale > 25)
        scale = 25;

    Vector<float,3> tw = 0.0;
    for(int i = 0; i < SMOOTH_BUFFER; i++)
    {
        tw += w[i];
    }

    tw /= SMOOTH_BUFFER;

    if(yawing)
    {
        heading += tw[0]*delta*15;
    }
    else
    {
        pitch = tw[1]*1.57;
        roll = tw[0]*1.57;
    }

    lastTime = now;
}

void gz4d::Dowii::OnAcceleration(unsigned short id, Vector<float,3> a)

{
    w[wi] = a;
    wi++;
    if (wi >= SMOOTH_BUFFER)
        wi = 0;
}
