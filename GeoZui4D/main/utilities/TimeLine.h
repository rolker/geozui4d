/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// TimeLine.h

#ifndef TIMELINE_H
#define TIMELINE_H

//class TimeLine;

#include "objects/Line.h"
#include "utilities/TimeSequence.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableWrapperSupport.h"

//class Line;

class TimeLineNode
{
public:
    virtual ~TimeLineNode() {};
    LineNodeType *lineNode;
};


/** 
 *  Class for maintaining a track of locations that can be represented
 *  graphically with a line.  
 *  \par Purpose:
 *  The TimeLineNode data type is intended to be derived by child classes to
 *  facilitate the addition of more information.  The destructor for this 
 *  class will attempt to destroy any data members left in the list as 
 *  being of type TimeLineNode.
 *  
 *  \author Matthew Plumlee (mdp) 
 *  \date 12/9/2004
 */
///////////////////////////////////////////////////////////////////////////////
class TimeLine: public TimeSequence, public PosableWrapperSupport
{
  public:
    TimeLine(Line * l = NULL);
    ~TimeLine();

    void insert(double t, double x, double y, double z);
    void insert(double t, gz4d::Point<double> const &point) {insert(t, point[X], point[Y], point[Z]);}
    LineNodeType *insertFromLine(double t, gz4d::Point<double> const &point, LineNodeType *&previous, LineNodeType *lnode);

    gz4d::Point<double> getPosition(); //returns position at current iteration node
    bool getPosition(double t, gz4d::Point<double> &point);

    void DrawLowResSection(float degradation, Window3D *win, double startTime, double endTime);
    void getHtmlInfo(std::stringstream &html, short level);

    Line * line;

  protected:
    TimeSequenceNode *insert(double t, TimeLineNode *info, double x, double y, double z);
    void removeFromLine(TimeSequenceNode *c);
    TimeLineNode *getPosition(double t, double &weight, TimeLineNode *&i2, gz4d::Point<double> &point);

    TimeLineNode *indexNodeAt(int idx);
    void resetIndexedNode() {nodeIndex = -1; indexedNode = NULL;}
    TimeSequenceNode *indexedNode; //to speed repetitive or sequential indexing
    int nodeIndex;

    //Wrapper Support
    virtual GLboolean indexContentsModifiable(PosableStateWrapper *wrapper);
    virtual void wrapperChangedValue(PosableStateWrapper *wrapper, long &changeMask); //wrapper changed something
    virtual void updateWrapper(PosableStateWrapper *wrapper, long &changeMask); //we're updating the wrapper
    virtual PosableWrapperList *findWrapperNode(PosableStateWrapper *wrapper, GLboolean mustExist = GL_TRUE, GLboolean getParentNode = GL_FALSE);

    //data members
    PosableWrapperList wrappers;

    TimeLineNode *selectedNode;
    int selectedNodeIndex;
};

#endif
