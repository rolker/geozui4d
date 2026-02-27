#include "MPHandle.h"
#include "MPCommand.h"
#include "interactors/PlanarPointHandle.h"
#include "engine/PosableStateBase.h"

extern PointHandle *pointHandle;

REGISTER_OBJECT(MPHandle, "");

MPHandle::MPHandle()
{
}

MPHandle::MPHandle(int id, gz4d::xml::Node cmd)
    :MPPoint(id,cmd,PICK_CLICK1 | PICK_PRESS3)
{
    allowBaseExternalEdit = true;
    isEditableFlag = true;
    respondToPress = true;
    inConstructor = true;
    MPHandle::command(cmd, std::string());
    inConstructor = false;

}

void MPHandle::command(gz4d::xml::Node c, std::string const &cmd)
{
	if(!inConstructor)
		MPPoint::command(c, cmd);
/*	c.childNode();
	while(c.hasCurrentNode())
	{
		if(c.nodeName() == "orientation")
		{
			float heading;
			if(c.getFloat(heading,"h"))
			{
				pose->setOrientation(0.0,heading);
				if(inConstructor)
					pointHandle->setDrawMode(PlanarPointHandle::DrawRotationTools);
			}
		}
		c.nextNode();
	}
	c.parentNode();*/
}

void MPHandle::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
    if (pose == whichPose)
    {
        ((MPCommand*)parent)->itemMoved(itemID,pose->getPosition());
    }
    ObjectCollection::PoseChanged(whichPose, whatChanged);
}
