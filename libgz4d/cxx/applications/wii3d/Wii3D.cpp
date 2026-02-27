#include "Wii3D.h"

#include <iostream>
#include <gz4d/time.h>
#include <GL/glut.h>

Wii3D::Wii3D()
{
    heading = 0;
    scale = 1.0;
    pitch = 0.0;
    roll = 0.0;
    xpos = ypos = zpos = 0.0;
    vx = vy = .5;
    lastTime = gz4d::time::Now();
    for(int i = 0; i < SMOOTH_BUFFER; i++)
    {
        n[i] = 0.0;
        w[i] = 0.0;
    }
    ni = 0;
    wi = 0;

    showTop = true;
    showBottom = true;
    translating = false;
}

Wii3D::~Wii3D()
{
}

void Wii3D::OnButton(unsigned short id, unsigned short b, bool down)
{
    if (b == WIIMOTE_BUTTON_H)
        exit(0);
    if (b == WIIMOTE_BUTTON_P && down)
        vsize *= 1.25;
    if (b == WIIMOTE_BUTTON_M && down)
        vsize *= 0.8;
    if (b == WIIMOTE_BUTTON_1 && down)
        showTop = !showTop;
    if (b == WIIMOTE_BUTTON_2 && down)
        showBottom = !showBottom;
    if (b == WIIMOTE_BUTTON_U)
        rumble(id,.2);
    if (b == WIIMOTE_BUTTON_D)
        rumble(id,.4);

}


void Wii3D::OnNunchukButton(unsigned short id, unsigned short b, bool down)
{
    std::cerr << b << ',' << down << std::endl;

    if (b == WIIMOTE_NUNCHUK_BUTTON_Z)
        translating = down;
}

void Wii3D::OnIdle()
{
    double now = gz4d::time::Now();
    double delta = now-lastTime;

    heading += j[0]*180*delta;
    scale *= 1-j[1]*delta*5;
    if (scale < 0.1)
        scale = 0.1;
    if (scale > 25)
        scale = 25;

    gz4d::Vector<float,3> tn,tw;
    for(int i = 0; i < SMOOTH_BUFFER; i++)
    {
        tn += n[i];
        tw += w[i];
    }

    tn /= SMOOTH_BUFFER;
    tw /= SMOOTH_BUFFER;

    if(!translating)
    {
        pitch = tw[1] * 75.0;
        pitch -= 15.0;

        roll = tw[0] * 75.0;
    }
    else
    {
        zpos += tw[1]*delta;
        xpos += tw[0]*delta;
    }

    vx = (tw[0]+1)/2.0;
    vy = (tw[1]+1)/2.0;

    lastTime = now;
}

void Wii3D::OnNunchukAcceleration(unsigned short id, gz4d::Vector<float,3> a)
{
        n[ni] = a;
        ni++;
        if (ni >= SMOOTH_BUFFER)
            ni = 0;
}

void Wii3D::OnNunchukJoystick(unsigned short id, gz4d::Vector<float,2> js)
{
    j = js;
    if (js[0] < 0.03 && js[0] > -0.03)
        j[0] = 0.0;
    if (js[1] < 0.03 || js[1] > -0.03)
        j[1] = 0.0;
}

void Wii3D::OnAcceleration(unsigned short id, gz4d::Vector<float,3> a)
{
        w[wi] = a;
        wi++;
        if (wi >= SMOOTH_BUFFER)
            wi = 0;
}

