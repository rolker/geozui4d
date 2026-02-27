/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef MPDISPLAYTYPE_H
#define MPDISPLAYTYPE_H

//#include<GL/glew.h>
#include "engine/ObjectCollection.h"
#include "objects/MissionPlan/MissionPlan.h"
#include <map>
#include "engine/Interactor.h"

namespace gz4d
{
    class Xml;
};

class MPItemBase;

/// \ingroup MissionPlanGroup
/// Base for display elements of an AUV mission plan.
/// Gets attached to a command instance. Displays the  
/// command and allows the user to interact with various
/// control points to modify the command's  parameters.
class MPCommand: public ObjectCollection
{
public:
	MPCommand();
    MPCommand(int cid, gz4d::xml::Node initcmd);
	DERIVE_OBJECT(MPCommand);
	
	//virtual void DrawLowRes(float degradation, Window3D *win) = 0;
	int GetCmdID(){return cmdID;}
	virtual void command(gz4d::xml::Node c);
	static void getColor(gz4d::xml::Node node, float * const color);
	int HandleInput(unsigned int inputType, InputInfo *info);
    void itemSelected(int item, bool s= true);
    bool isSelectedCmd(){return selectedCmd;};
    void itemMoved(int id, gz4d::Point<double> const &p);
protected:
	
	int cmdID ; ///< Id assigned to command from python side
	bool selectedCmd; ///< True if we are the currently selected command
	float color[4];
	float selectedColor[4];
	
        std::map<int, MPItemBase *> items;
};

#endif
