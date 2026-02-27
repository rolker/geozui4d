/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/


#ifndef MISSIONPLAN_H
#define MISSIONPLAN_H

#include "engine/ObjectCollection.h"
#include <map>

class MPCommand;

/**
Generic AUV mission planner. 
MissionPlan displays elements for missionPlanner.py
*/
class MissionPlan: public ObjectCollection
{
public:
    MissionPlan();
    ~MissionPlan();
    DERIVE_OBJECT(MissionPlan);
    
    int DrawHighRes(int eye, Window3D *win);
    void DrawLowRes(float degradation, Window3D *win);
    void PoseChanged(PosableStateBase *whichPose, long whatChanged);
    virtual void Idle(double time);
    //int HandleInput(unsigned int inputType, InputInfo *info);
        int pickAction (Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);

        void getHtmlInfo(std::stringstream &html, short level);
    
        void sendToGUI(std::string msg);
    void sendPosition(int id);
    
    void cmdSelected(MPCommand * newSelected);
private:
    void drawGrid();
    void drawGeoRefPoint();
    
    std::map<int, MPCommand> commands; ///< Associative array of MPCommands (like a dict in python)
    MPCommand * selectedCmd;
};


#endif
