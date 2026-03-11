// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Vip.h
/*---------------------------------------------------------------------------*/
/** 
 *  This object is used to visualize Ausi vips (auvs, instrument platforms, etc)
 *  
 *
 *  Created by rja June 13th, 2001
 *  Modifications to behavior interface and haptics by Rick Komerska, Summer 2003
 */
/*---------------------------------------------------------------------------*/
class Vip;

#ifndef VIP_H
#define VIP_H

#include "engine/ObjectCollection.h"
#include "interactors/Pickable.h"
#include "engine/Tail.h"
//#include "utilities/LinkedList.h"
//#include "VIP_Behavior.h"
#include "objects/GeoData/DataPoints.h"

#include "engine/RTSource.h"
#include <filesystem>

class PosableStateLock;
class DataLines;
class DSL;

class Vip : public ObjectCollection
{
  public:
    Vip();
    ~Vip();
    DERIVE_OBJECT(Vip);

    //File-related functions
    int readFrom(FILE *file); //read from a file
    int processField(int option, char **fields);

    int writeTo(FILE *file);  //define this if you want to be able to save to a file
    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

    //Display-related functions
    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);

    //Picking functions
    virtual int  pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);
//    virtual void renderPickableObject(Window3D *inWindow);

    //Update functions
    virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);
    void Idle(double time);

    //Scripting interface
    virtual void ShowInfo(GLuint *names = NULL, int numNames = 0);
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);

    //Behavior i/f
/*  int attachCurrentBehavior(VIP_Behavior* beh);
    void detachCurrentBehavior();
    void addBehavior(VIP_Behavior* current_beh, VIP_Behavior* new_beh);
    void deleteCurrentBehavior();
    void updateBehaviors();
    void getPathPointPosition(int i, Point3d& pos, int behavior_index = 0);
    void editPathPoint(int i, const Point3d val, bool isEndPt = false, int behavior_index = 0);
    void addPathPoint(int p, int behavior_index = 0);
    void deletePathPoint(int p, int behavior_index = 0);
    int getCurrentBehaviorIndex() const {return currentBehaviorIndex;}
    void deleteMission();
    float getMinSpeed() const;
    float getMaxSpeed() const;
    void writePath(ostream& stream);*/


  protected:
    Tail tail;
    int type, subtype;

    enum VipParams {NAME = FirstObjectKeyword,
                    SPEED,
                    VEHICLE, INSTRUMENT, PLATFORM,  // types
                    OPEN_SPACE_FRAME, CUSTOM_SPACE_FRAME, TORPEDO, SOLAR_AUV, CRAWLER, MOORING, ABE}; // subtypes

    enum DataTypes {Unknown = 0, Remus};

    //Behaviors
/*  VIP_Behavior *currentBehavior, *nextBehavior;
    int currentBehaviorIndex, nextBehaviorIndex;
    LinkedList behaviorList;
    DataLines* vip_path;
    float minSpeed, maxSpeed;*/

    Point3d baseGeoRef;
    bool first_time;

    // real time nav
    DSL * dsl;
    std::string dslObject;

    // custom space frame
    float width, height, length;

    //remus
    std::string datafilename;
    int datatype;
    FILE * datafile;


};

#endif
