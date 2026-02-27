/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ObjectHistory.cpp
/** 
 *	Class for maintaining a history of locations and orientations that can be
 *	represented graphically with a line and followed by an object.  The
 *	PoseSnapshotNode data type is intended to be derived by child classes to
 *	facilitate the addition of more information.
 *  
 *	Created by Matthew Plumlee 12/9/2004
 */
///////////////////////////////////////////////////////////////////////////////
#include "utilities/ObjectHistory.h"

#include "utilities/Utilities.h"
#include "engine/InputMediator.h"
#include "engine/Scene3D.h"
#include "utilities/DottedAddress.h"
#include "engine/ObjectCollection.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
ObjectHistory::ObjectHistory(Object * owner)
    :timeOffset(0.0)
{
    trackedObject = owner;
    line->drawTube(false);
    visible = false;
    startShow = endShow = 0.0;
    startHighlight = endHighlight = 0.0;
}

/*---------------------------------------------------------------------------*/
ObjectHistory::~ObjectHistory()
{
	PoseSnapshotNode *doomed;

	iterator = start;
	while (iterator)
	{
		doomed = (PoseSnapshotNode*) iterator->data;
		if (doomed)
			delete doomed;
		iterator->data = NULL;
		iterator = iterator->next;
	}
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void ObjectHistory::insert(double t, PoseSnapshotNode *info, double x, double y,
						   double z, OrientationInfo &o, double altitude)
{
	OrientationInfo *io = &info->orientation;
	io->type = o.type;
	io->angle = o.angle;
	io->axis[Yaw] = o.axis[Yaw];
	io->axis[Pitch] = o.axis[Pitch];
	io->axis[Roll] = o.axis[Roll];
	
	info->altitude = altitude;

	TimeLine::insert(t, info, x, y, z);
}

/*---------------------------------------------------------------------------*/
void ObjectHistory::insert(double t, double x, double y, double z, OrientationInfo &o, float altitude)
{
	PoseSnapshotNode *info = new PoseSnapshotNode;
	insert(t, info, x, y, z, o, altitude);
}

/*---------------------------------------------------------------------------*/


/**	returns orientation at current iteration node
 */
const OrientationInfo *ObjectHistory::getOrientation()
{
	if (iterator && iterator->data)
		return &((PoseSnapshotNode *)(iterator->data))->orientation;
	else
		return NULL;
}

/*---------------------------------------------------------------------------*/
/**	returns position and orientation at time t
 */
bool ObjectHistory::getPositionOrientation(double t, gz4d::Point<double> &point, OrientationInfo &o, double &alt)
{
	PoseSnapshotNode *info2;
	double weight;

	if (getPositionOrientation(t, weight, info2, point, o, alt))
		return true;
	return false;
}

/*---------------------------------------------------------------------------*/
/**	returns position and orientation at time t with support information to
 *	make interpolations on other data actually in the derived TimeLineNode
 *	data type.
 */
PoseSnapshotNode *ObjectHistory::getPositionOrientation(double t, double &weight,
        PoseSnapshotNode *&info2, gz4d::Point<double> &point,
									OrientationInfo &o, double &alt)
{
	PoseSnapshotNode *info1;
	TimeLineNode *v;
	OrientationInfo *o1, *o2;

	info1 = (PoseSnapshotNode*)getPosition(t+timeOffset, weight, v, point);
	if (!info1)
		return NULL;
	info2 = (PoseSnapshotNode*) v;
	
	if(info1->altitude >= 0)
		if(info2->altitude >= 0)
			alt = info1->altitude+((info2->altitude-info1->altitude)*weight);
		else
			alt = info1->altitude;
	else if(info2->altitude >= 0)
		alt = info2->altitude;
	else
		alt = -1.0;

	o1 = &info1->orientation;
	o2 = &info2->orientation;
	o.type = o1->type;
	
	if (o.type == OrientationInfo::typeYawPitchRoll)
	{
		//o.axis[Yaw] = o2->axis[Yaw];
		o.axis[Yaw] = (float)interpolateDegrees(o1->axis[Yaw], o2->axis[Yaw], weight);
		o.axis[Pitch] = (float)interpolateDegrees(o1->axis[Pitch], o2->axis[Pitch], weight);
		o.axis[Roll] = (float)interpolateDegrees(o1->axis[Roll], o2->axis[Roll], weight);
	}
	else // we need to look into proper quat interp if we start using them!
		o.angle = (float)interpolateDegrees(o1->angle,o2->angle,weight);
		

	return info1;
}

void ObjectHistory::Draw(double time, float degradation, Window3D *win)
{
    if (!visible)
        return;

    LineNodeType* startNode = NULL;
    LineNodeType* endNode = NULL;

    if(startShow != 0.0)
        startNode = ((TimeLineNode*)get(time + timeOffset + startShow))->lineNode;

    if(endShow != 0.0)
        endNode = ((TimeLineNode*)get(time + timeOffset  + endShow))->lineNode;

    line->DrawLowResSection(degradation,win,startNode,endNode);

}

void ObjectHistory::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">ObjectHistory</h" << level << ">";
    if (visible)
        html << "Path is on, turn <a href=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/visible/off\">off</a><br>";
    else
        html << "Path is off, turn <a href=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/visible/on\">on</a><br>";

    html << "<form action=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/start/sTime\">";
    html << "Path start offset <input type=\"text\" name=\"sTime\" value=\"" << startShow <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/end/eTime\">";
    html << "Path end offset <input type=\"text\" name=\"eTime\" value=\"" << endShow <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/timeOffset/tOffset\">";
    html << "Path time offset <input type=\"text\" name=\"tOffset\" value=\"" << timeOffset <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/color/rValue/gValue/bValue/aValue\">Path color";
    html << " r:<input type=\"text\" name=\"rValue\" value=\"" << line->getColor()[0] <<"\"/>";
    html << " g:<input type=\"text\" name=\"gValue\" value=\"" << line->getColor()[1] <<"\"/>";
    html << " b:<input type=\"text\" name=\"bValue\" value=\"" << line->getColor()[2] <<"\"/>";
    html << " a:<input type=\"text\" name=\"aValue\" value=\"" << line->getColor()[3] <<"\"/>";
    html << "<input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/startHighlight/sTime\">";
    html << "Path start highlight <input type=\"text\" name=\"sTime\" value=\"" << startHighlight <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/endHighlight/eTime\">";
    html << "Path end offset <input type=\"text\" name=\"eTime\" value=\"" << endHighlight << "\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << trackedObject->GetId() << "/path/highlightColor/rValue/gValue/bValue/aValue\">Highlight color";
    html << " r:<input type=\"text\" name=\"rValue\" value=\"" << line->getColor()[0] <<"\"/>";
    html << " g:<input type=\"text\" name=\"gValue\" value=\"" << line->getColor()[1] <<"\"/>";
    html << " b:<input type=\"text\" name=\"bValue\" value=\"" << line->getColor()[2] <<"\"/>";
    html << " a:<input type=\"text\" name=\"aValue\" value=\"" << line->getColor()[3] <<"\"/>";
    html << "<input type=\"submit\" value=\"Modify\"/></form><br>";

    TimeLine::getHtmlInfo(html,level);
}

int ObjectHistory::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    bool ok = false;
    bool redraw = false;
    std::stringstream ret;

    if(argc >= 1)
    {
        std::string cmd(argv[0]);
        if (cmd == "visible")
        {
            if (argc == 2)
            {
                visible = isTrue(argv[1]);
                redraw = true;
            }
            ret << visible;
            ok = true;
        }
        if (cmd == "start")
        {
            if (argc == 2)
            {
                startShow = atof(argv[1]);
                redraw = true;
            }
            ret << startShow;
            ok = true;
        }
        if (cmd == "end")
        {
            if (argc == 2)
            {
                endShow = atof(argv[1]);
                redraw = true;
            }
            ret << endShow;
            ok = true;
        }
        if (cmd == "timeOffset")
        {
            if (argc == 2)
            {
                timeOffset = atof(argv[1]);
                redraw = true;
                trackedObject->getParent()->setBBoxStale();
            }
            ret << timeOffset;
            ok = true;
        }

        if (cmd == "color")
        {
            if (argc == 5)
            {
                gz4d::Color color;
                color[0] = atof(argv[1]);
                color[1] = atof(argv[2]);
                color[2] = atof(argv[3]);
                color[3] = atof(argv[4]);
                line->setColor(color);
                redraw = true;
            }
            ret << line->getColor()[0] << ", " << line->getColor()[1] << ", " << line->getColor()[2] << ", " << line->getColor()[3];
            ok = true;
        }
        if (cmd == "highlightColor")
        {
            if (argc == 5)
            {
                gz4d::Color color;
                color[0] = atof(argv[1]);
                color[1] = atof(argv[2]);
                color[2] = atof(argv[3]);
                color[3] = atof(argv[4]);
                line->setHighlightColor(color);
                redraw = true;
            }
            ret << line->getHighlightColor()[0] << ", " << line->getHighlightColor()[1] << ", " << line->getHighlightColor()[2] << ", " << line->getHighlightColor()[3];
            ok = true;
        }
    }
    if (ok)
    {
        if(redraw)
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
        Tcl_SetResult(interp, (char *)(ret.str().c_str()), TCL_VOLATILE);
        return TCL_OK;
    }
    return TCL_ERROR;
}

