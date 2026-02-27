/* Copyright (c) 2008 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
// Feature.h: Class to work with X3D Feature spec for the Digital Coast Pilot.
//            Feature acts like a LOD node that is based on view direction
//            rather than scale.
//
//////////////////////////////////////////////////////////////////////

#ifndef FEATURE_H
#define FEATURE_H

#include "engine/ObjectCollection.h"
#include <gz4d/x3d/X3DNode.h>

class Feature: public ObjectCollection
{
public:

    DERIVE_OBJECT(Feature);

    Feature(GLboolean allocatePosable = GL_TRUE, int pickingTypes = PICK_CLICK1 | PICK_PRESS3);

    //We want DrawLow/HighRes to only draw one child, based on viewing direction.
    virtual void DrawLowRes(float degradation, Window3D *win);
    virtual int DrawHighRes(int eye, Window3D *win); //return 1 if done, 0 if not done.

	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

    void setPriority(float p);
    void setMinRange(float p);
    void setMaxRange(float p);
    void setEmphasisFactor(float p);
    void setScale(gz4d::SFVec3f scale);
    void setRange(gz4d::SFVec3f range);
    void setLineColor(gz4d::SFColor color);
    void setLocationUTM(gz4d::SFVec3d location);
    void addChild(gz4d::SFNode child);

    void updateChildren();

protected:
    Object *chooseObjectToDisplay(Window3D *win);
    void determineChildTransform(Window3D *win);
    virtual void preTransformChild();

    float priority;
    static const float minEmphasisTFactor, maxEmphasisTFactor, emphasisMagic;
    float minRange, maxRange, ownEmphasisFactor;
    gz4d::SFVec3f scaleChildren;
    gz4d::SFVec3f priorityRange;
    gz4d::SFColor lineColor;

    //variables to help keep track of which child to draw
    std::vector<float> yawBoundaries; //0 is boundary between last and first, 1 between first and second...
    std::vector<Object *> objectsByBounds;
    int lastBoundaryIndex;
    double approachAngle, approachDistance;

    float scaleFactor;
    float viewPitch;
    Object *child;
    static float emphasisFactor, CriticalPitch;
};


#endif
