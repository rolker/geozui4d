// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// NavigationPath.h
/** 
 *  This class helps the user create a path plan on the water surface.  It
 *  supports operations on waypoints (changing speed, location, notes, etc.)
 *  and also provides a mechanism for other classes to query it to determine
 *  at what point a vessel is planned to arrive at particular locations
 *  (for instance: Gutm's will color themselves according to the expected
 *  water level at the time a vessel is planned to transit over a given grid
 *  cell).
 *
 *  Note: times stored in the NavigationPath are estimated times of arrival.
 *  
 *  Originally created by Roland Arsenault (some time in winter 2002/2003)
 *  Revamped by Matthew Plumlee 12/16/2004
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef NAVIGATIONPATH_H
#define NAVIGATIONPATH_H
//class NavigationPath;

#include "utilities/TimeLine.h"
#include "engine/Object.h"
#include "Vessel.h"

class ValueDrag;

class WaypointNode: public TimeLineNode
{
  public:
    float heading, sin_h, cos_h;
    Point2d port, starboard;
    float speed;
    float distanceToNext;
    float timeToGo;
    float eta;
    std::string label;
    float cost;
    int row;
};

class NavigationPath: public ObjectCollection, public TimeLine //public Interactor
{
  public:
     
    DERIVE_OBJECT(NavigationPath);
    NavigationPath();
    ~NavigationPath();

    int readFrom(FILE *file);
    int writeTo(FILE *file);
//  int DrawHighRes(int eye, Window3D * win);
    void DrawLowRes(float degradation, Window3D * win);
//  int HandleInput(unsigned int inputType, InputInfo * info);
    virtual void Idle(double time);
    Object * LoadXML(const char * fname);
    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    virtual void SaveXML(FILE * out, std::string const &path);
    virtual void SaveXMLattributes(FILE * out);

    //Picking
        virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);

    double getTimeAt(gz4d::Point<double> const &p); //if point is along path (in port-starboard bounds), return eta to point.

    WaypointNode *insert(double t, double x, double y, double z);
    WaypointNode *insert(double t, const double *point) {return insert(t, point[X], point[Y], point[Z]);}

    WaypointNode *getWaypoint(double t = std::numeric_limits<double>::quiet_NaN()); //returns waypoint at current iteration node or the one nearest & previous to the passed time
    bool getWaypoint(double t, WaypointNode *waypoint);  //fills passed waypoint structure with interpolated info for given time.

    //Wrapper Support
    virtual GLboolean insertValue(PosableStateWrapper *wrapper, GLboolean beforeCurrent = GL_TRUE, int dim = 0);
    virtual GLboolean deleteValue(PosableStateWrapper *wrapper, int dim = 0);

    //Scripting support
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);

    //GUTM support
    void updateTimes(double *plannedPathTime, const double *pos, float cellsize, int maxx, int maxy, int &navPathUpdateCounter);

    std::string getWaypointLabel(WaypointNode const &cursor);
    void formatSeconds(float secs, char* value);
    
    Object * object;
    const char * file;
    Vessel * vessel;
    float currentHeading;


  protected:
    virtual void SaveXMLnodeAttributes(FILE * out, TimeLineNode *node);
    void updatePath(TimeSequenceNode *becauseOfMe = NULL);
    void updateEntirePath();
    void updateCorridor(TimeSequenceNode *startAt = NULL, TimeSequenceNode *endAt = NULL);
    void insert(double t, WaypointNode *waypoint, double x, double y, double z, bool loading = false);
    WaypointNode *getWaypoint(double t, double &weight, WaypointNode *&i2, gz4d::Point<double> &position);

    //Wrapper Support
    virtual void wrapperChangedValue(PosableStateWrapper *wrapper, long &changeMask); //wrapper changed something

    //Talkin' to Tcl
    GLboolean tellTclWeExist(Tcl_Interp *interp = NULL);
    GLboolean fillTclArray(Tcl_Interp *interp = NULL);
    GLboolean updateFromTclTable(Tcl_Interp *interp, const char *varName, const char *index);
    GLboolean insertRow(Tcl_Interp *interp, WaypointNode *waypoint);
    GLboolean deleteRow(Tcl_Interp *interp, int row);
    GLboolean deletingPoint;

    float corridorWidth, defaultSpeed, predictedPosition[2], zlevel, etaCumulative;
    float dayCostRate, fuelCostRate, totalCost; 
    double totalDistance, currentGzTime;
    int rowCount;
    int updateCounter;

    bool pathNeedsFullUpdate;
    ValueDrag *valueDrag;

    Tcl_Interp *local_interp;
    GLboolean initializedWithTcl, tclTableNeedsUpdating;
    
    TimeSequenceNode *mostRecentNode;
};

#endif
