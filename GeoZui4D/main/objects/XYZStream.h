/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef XYZSTREAM_H
#define XYZSTREAM_H

#include "GutmTree.h"

#include "SoundingBuffer.h"
#include "engine/ObjectCollection.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <gz4d/Point.h>
#include <gz4d/time.h>

class Window3D;

#define INBUFSIZE 66000

typedef Point3f *PointStream;

/**
 *This class makes any x,y,z data into gridded data.
 *\author Roland Arsenault (rja)
 *\date 2001
*/
class XYZStream : public ObjectCollection
{
public:
    XYZStream();
    ~XYZStream();
    DERIVE_OBJECT(XYZStream);

    void Initialize(double resolution,std::string type,double rad = 0.0);
    void InitFile(std::string const &fn, std::string type, double res, double rad);

    int readFrom(FILE * file);
    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    void Idle(double time);
    void LoadNext();
    void AddPoint(double lat, double lon, gz4d::Point<float> const &p, gz4d::Time t, float value, float rad = -1.0);
    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);
    void ShowInfo(GLuint *names, int numNames);
    int writeTo(FILE * outfile);
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);
    void HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data);
private:
    GutmTree * gtree;

    long int pointcount;
    unsigned char degres;
    int degup,degdown;
    int degcount;  // used to smooth out deg problems.
    float resolution;
    int maxdeg;

    int showquad; // flag to indicate if quadtree structure should be displayed
    int paused;  // used to pause file loading

//	paramaters used for weighted averages
    double radius;

    FILE * ssfile;
    std::string ftype;
    bool posd; // positive depth

    Point3d lastPos;
    Point3d lastPing;
    int drawLine;
    bool altData;
};


#endif
