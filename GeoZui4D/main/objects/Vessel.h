/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Vessel.h
/*---------------------------------------------------------------------------*/
/** 
 *	This object is used to visualize ships, potentially with shadows.
 *	
 *
 *	Created by mdp 1/28/2003
 *	Rederived from BasicVessel 9/8/2004
 */
/*---------------------------------------------------------------------------*/
class Vessel;

#ifndef VESSEL_H
#define VESSEL_H

#include<gz4d/glo.h>
#include "BasicVessel.h"
#include "interactors/Pickable.h"
#include "engine/Tail.h"
#include <map>

class PosableStateLock;
class Gps;
class Ais;
class NmeaSource;
class VesselHistory;
class VesselSnapshotNode;
class VesselPredictor;
class DataLines;

class Vessel : public BasicVessel
{
public:
    Vessel();
    ~Vessel();
    DERIVE_OBJECT(Vessel);

    //File-related functions
    int readFrom(FILE *file); //read from a file
    int processField(int option, char **fields);
    //int writeTo(FILE *file);  //define this if you want to be able to save to a file
    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

    //Display-related functions
    void DrawLowRes(float degradation, Window3D *win);

    //Update functions
    virtual void Idle(double time);

    //Scripting interface
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);

    // Vessel Position & orientation saved in detailHistory (VesselHistory)
    void SetVesselPositionOrientation(double x, double y, double z,OrientationInfo o,double t);


protected:
    struct DataSource
    {
        NmeaSource * nmea;
        std::string address;
        std::string filename;
        int port;
        bool udp;
        bool preload;
        Point3f offset;
        bfs::path logfile;
        std::string prefix;
        bool filterIn;
        DataSource():port(0),udp(false),preload(false),filterIn(true){offset[X] = offset[Y] = offset[Z] = 0.0;}
    };

    typedef std::map<std::string,DataSource> SourceMap;
    SourceMap sources;


    float sounder_depth;
    float newSounderDepth;

    float rudder_angle;
    float newRudderAngle;

    float priorSpeed;

    VesselPredictor *predictor;
    GLboolean showPredictor;

    Point3f anchor_offset;
    Point3d anchor_position;

    std::shared_ptr<VesselHistory> detailHistory; // allows for any vessel to hook into the time mechanism
    VesselSnapshotNode *latestVesselNode;

    float currentDirection, currentStrength;

    float xError, yError;
    DataLines *crosstrackPath;
    int showCrosstrack;
    unsigned int crosstrackIndex; //keeps track of intersection point

    FILE *dumplogFile;

    bool calculateTrackLocation(float posx, float posy, float h, float &XTEx, float &XTEy, float &headingDifference);

    enum VesselParams {GPS_ADDRESS = FirstVesselKeyword, GPS_FILENAME, GPS_PORT, GPS_OFFSET, ANCHOR_OFFSET, GPS_UDP};
};

#endif
