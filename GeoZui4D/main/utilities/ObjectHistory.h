/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ObjectHistory.h

#ifndef OBJECTHISTORY_H
#define OBJECTHISTORY_H
//class ObjectHistory;

#include "utilities/TimeLine.h"
#include "engine/PosableState.h"

class PoseSnapshotNode: public TimeLineNode
{
  public:
    OrientationInfo orientation;
    float altitude;
};

/** 
 *  Class for maintaining a history of locations and orientations that can be
 *  represented graphically with a line and followed by an object.  
 *  \par Purpose:
 *  The PoseSnapshotNode data type is intended to be derived by child classes to
 *  facilitate the addition of more information.  
 *  \note The destructor for this class will attempt to destroy any data members 
 *  left in the list as being of type PoseSnapshotNode.
 *  
 *  \author Matthew Plumlee (mdp)
 *  \date 12/9/2004
 */
///////////////////////////////////////////////////////////////////////////////

class ObjectHistory: public TimeLine
{
    public:
        ObjectHistory(Object * owner);
        ~ObjectHistory();

        void Draw(double time, float degradation, Window3D *win);

        void insert(double t, double x, double y, double z, OrientationInfo &o, float altitude = -1.0);
        void insert(double t, const double *point, OrientationInfo &o) {insert(t, point[X], point[Y], point[Z], o);}

        const OrientationInfo *getOrientation(); //returns orientation at current iteration node
        bool getPositionOrientation(double t, gz4d::Point<double> &point, OrientationInfo &o, double & alt);
        void getHtmlInfo(std::stringstream &html, short level);
        double getTimeOffset() const {return timeOffset;}

        void setVisible(bool v=true){visible = v;};
        bool isVisible(){return visible;};
        int perform(int argc, const char **argv, Tcl_Interp *interp);

        void setStartShow(double s){startShow = s;}
        void setEndShow(double e){endShow = e;}
        void setStartHighlight(double s){startHighlight = s;}
        void setEndHighlight(double e){endHighlight = e;}
    protected:
        void insert(double t, PoseSnapshotNode *info, double x, double y, double z, OrientationInfo &o, double altitude);
        PoseSnapshotNode *getPositionOrientation(double t, double &weight, PoseSnapshotNode *&i2, gz4d::Point<double> &point, OrientationInfo &o, double &alt);

        bool visible;
        double startShow;     ///< Offset from current time where path starts.
        double endShow;       ///< Offset from current time path ends.
        double startHighlight; ///< Offset from current time where highlight starts.
        double endHighlight;   ///< Offset from current time where highlight ends.
        Object * trackedObject;
        double timeOffset; ///< Number of seconds in which to offset the whole history
};

#endif
