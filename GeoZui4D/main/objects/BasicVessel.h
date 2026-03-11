// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// BasicVessel.h
/*---------------------------------------------------------------------------*/


class BasicVessel;

#ifndef BASICVESSEL_H
#define BASICVESSEL_H

#include "engine/ObjectCollection.h"
#include "interactors/Pickable.h"
#include "engine/Tail.h"
#include <gz4d/socket/UDPSocket.h>
#include <gz4d/TimeSequence.h>

class PosableStateLock;
/** 
 *  This object is used to visualize ships, potentially with shadows.
 *  \note Adapted from Vessel (for vessels w/out real time input)
 *  \author Matt Plumlee (mdp)
 *  \date 8/19/2004
 */
/*---------------------------------------------------------------------------*/
class BasicVessel : public ObjectCollection
{
public:
    BasicVessel();
    ~BasicVessel();
    DERIVE_OBJECT(BasicVessel);
    virtual void Idle(double time);

    //Geometry-related functions
    enum {BOW, HULL, STERN, BRIDGE, NUM_PARTS};
    void setWidth(int part, float val) {width[part] = val;}
    void setLength(int part, float val) {length[part] = val;}
    void setHeight(int part, float val) {height[part] = val;}
    void setOffset(int part, float val) {offset[part] = val;}
    void establishGPSoffset(float *offset_amount);
    void calculateOffsets(float bridge_nudge = 0.0f);

    //Info functions
    virtual void ShowInfo(unsigned int *names = NULL, int numNames = 0);
    void setCallsign(std::string const &val) {callsign = val;}
    void setAisName(std::string const &val) {aisName = val;}
    void setDestination(std::string const &val) {destination = val;}

    //Course-related functions
    void setSpeed(float speed);
    void setCourse(float heading, GLboolean andHeadingToo = GL_FALSE);

    //File-related functions
    int readFrom(FILE *file); //read from a file
    int processField(int option, char **fields);
    //int writeTo(FILE *file);  //define this if you want to be able to save to a file
    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
        virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits) {return Object::pickAction(pickType, inputInfo, names, hits);}

    //Display-related functions
    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);
    void DrawAsFlatIcon(Window3D *win, float opaqueness = 1.0, GLboolean resizeWithScale = GL_TRUE);
    bool hideTail() {bool b = drawTail; drawTail = false; return b;}
    bool showTail() {bool b = drawTail; drawTail = true; return b;}
    bool hideCMG() {bool b = drawCMG; drawCMG = false; return b;}
    bool showCMG() {bool b = drawCMG; drawCMG = true; return b;}
    GLboolean hideChildren() {GLboolean b = drawChildren; drawChildren = false; return b;}
    GLboolean showChildren() {GLboolean b = drawChildren; drawChildren = true; return b;}
    void setModelWindow(Window3D *win);

    //Update functions
    virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);

    //Scripting interface
    virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    virtual int list(Tcl_Interp *interp);

protected:
    void establishOffset(PosableStateBase *&offsetPose, const char *offsetName, PosableStateLock *&lock, const char *lockName, const float *offset);
#define ESTABLISH_OFFSET(offsetName, offsetType, offset) establishOffset(offsetTo##offsetName, #offsetName, lock##offsetType##offsetName, "lockFrom" #offsetName, offset)

    //Strings
    //void setStringVal(char *&string, const char *val);
    //char *callsign, *aisName, *destination;
    std::string callsign, aisName, destination;

    Tail tail;
    bool drawTail, drawCMG, drawChildren;
    float width[NUM_PARTS], length[NUM_PARTS], height[NUM_PARTS];
    float offset[NUM_PARTS];
    Window3D *onlyModelWin;
    Object *shadowOver;
    float referenceLength;

  // target to be drawn as point when zoomed out
  float target_inner_size;
  float target_outer_size;
  float target_inner_color[3];
  float target_outer_color[3];

  float vesselColor[3];
  int toggle;

  double heading;

  GLboolean drawOpaque, cogPitch;

    //keep these so we can look at how another vessel sees the world
    PosableStateBase *courseMadeGood;
    PosableStateLock *lockFromCMG;
    GLboolean generatingCMG, generatingSOG;
    PosableStateBase *offsetToGPS, *offsetToBridge, *offsetToStern;
    PosableStateLock *lockToGPS, *lockFromBridge, *lockFromStern;
    Point3d lastPos;

    std::shared_ptr<gz4d::UDPSocket> telemetrySocket;

    gz4d::TimeSequence<float> speeds;
    bool showDepth;
    std::string speedUnits;

    virtual void initializingLabel();

    enum VesselParams {BOW_SIZE = FirstObjectKeyword, HULL_SIZE,
                   STERN_SIZE, BRIDGE_SIZE, SHADOW, FirstVesselKeyword};
};

#define BASICVESSEL_NAMES \
    OBJECT_NAMES, \
    "BOW_SIZE", "HULL_SIZE", \
    "STERN_SIZE", "BRIDGE_SIZE", "SHADOW"
#endif
