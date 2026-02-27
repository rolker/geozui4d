#ifndef WII3D_H
#define WII3D_H

#include <gz4d/WiimoteManager.h>

#define SMOOTH_BUFFER 25

/// Demonstration of Wiimote class usage by sub-classing.
class Wii3D: public gz4d::WiimoteManager
{
public:
    Wii3D();
    ~Wii3D();

    float heading;
    float scale;
    float pitch;
    float roll;
    bool showTop, showBottom;
    float vx,vy,vsize;
    float xpos, ypos, zpos;
    bool translating;

protected:
    void OnButton(unsigned short id, unsigned short b, bool down);
    void OnNunchukButton(unsigned short id, unsigned short b, bool down);
    void OnNunchukAcceleration(unsigned short id, gz4d::Vector<float,3> a);
    void OnNunchukJoystick(unsigned short id, gz4d::Vector<float,2> j);
    void OnAcceleration(unsigned short id, gz4d::Vector<float,3> a);
    void OnIdle();

    double lastTime;
    gz4d::Vector<float,2> j;
    gz4d::Vector<float,3> n[SMOOTH_BUFFER];
    int ni;
    gz4d::Vector<float,3> w[SMOOTH_BUFFER];
    int wi;
};

#endif
