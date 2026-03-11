// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// NavigationPath.cpp
/** 
 *  This class helps the user create a path plan on the water surface.  It
 *  supports operations on waypoints (changing speed, location, notes, etc.)
 *  and also provides a mechanism for other classes to query it to determine
 *  at what point a vessel is planned to arrive at particular locations
 *  (for instance: Gutm's will color themselves according to the expected
 *  water level at the time a vessel is planned to transit over a given grid
 *  cell).
 *  
 *  Originally created by Roland Arsenault (some time in winter 2002/2003)
 *  Revamped by Matthew Plumlee 12/16/2004
 */
///////////////////////////////////////////////////////////////////////////////
#include "GZincludes.h"
#include"objects/NavigationPath.h"
#include "interactors/PlanarPointHandle.h"
#include "utilities/TimePieces.h"
#include "utilities/Utilities.h"
//#include <time.h>
#include"engine/Window3D.h"
#include "engine/PosableStateWrapper.h"
#include "utilities/DottedAddress.h"
#include "interactors/ValueDrag.h"
#include "engine/TimeControl.h"
#include <gz4d/geo/Projection.h>

using namespace std;

//extern int dominantEye;
extern MainWindow * mainWindow;
extern PointHandle *pointHandle;

REGISTER_OBJECT(NavigationPath, "");

#define KNOTS_OVER_M_PER_S 1.945945946
#define METERS_OVER_MILES 1609.344

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
NavigationPath::NavigationPath()
: ObjectCollection(GL_TRUE, PICK_CLICK1 | PICK_PRESS1 | PICK_CLICK3)
//:Interactor("navPath")
{
    mainWindow->getScene3D()->SetNavPath(this);
    defaultSpeed = 2.57; // m/s (about 5 knots)
    corridorWidth = 100.0; // m
    totalDistance = 0.0;
    mostRecentNode = NULL;
    isEditableFlag = true;
    canUseInternalHandleFlag = GL_TRUE;
    allowBaseExternalEdit = false;
    line->deletingInfo = false;
    line->drawForPicking = false;
    zlevel = 0;
    local_interp = NULL;
    initializedWithTcl = GL_FALSE;
    deletingPoint = GL_FALSE;
    tclTableNeedsUpdating = GL_TRUE;
    etaCumulative = 0;
    totalCost = 0.0;
    dayCostRate = 0.3472; //$30K/day = 34.72 cents per second
    fuelCostRate = 0.01; //1 cent per second per velocity squared... which is who knows what.
    rowCount = 1;
    updateCounter = 0;
    object = NULL;
    localBBox = &bbox;

    pathNeedsFullUpdate = false;
    valueDrag = new ValueDrag(&corridorWidth);
    valueDrag->setFlagVariable(&pathNeedsFullUpdate);
    valueDrag->setValueAlwaysPositive();
                        
    file = Tcl_GetVar(mainWindow->interp,"vesselFile",TCL_GLOBAL_ONLY);
    if (file)
    {
        object = LoadXML(file);
        DYNAMIC_CAST(vessel,Vessel*,object);
    }

/*
  char tclstr[100];
    strcpy(tclstr,"source [file join $appdir \"scripts/object/NavigationPath.tcl\"]\n");

    mainWindow->Tcl_Eval(tclstr);
    
    globalInputMediator->PushInteractor(this);
 */
}

/*---------------------------------------------------------------------------*/
NavigationPath::~NavigationPath()
{
    WaypointNode *doomed;

    iterator = start;
    while (iterator)
    {
        doomed = (WaypointNode*) iterator->data;
        if (doomed)
        {
            delete doomed;
        }
        iterator->data = NULL;
        iterator = iterator->next;
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void NavigationPath::DrawLowRes(float degradation, Window3D * win)
{
    WaypointNode *cursor, *c_next;
    double t_now, t_next;
    gz4d::Point<double> pos_now, pos_next, pos;
    double midx, midy;
    double t;
    int idx;
    
    glPushAttrib(GL_POINT_BIT|GL_LIGHTING_BIT);
/*
    if(insertMode && waypointHead)
    {
        glLineWidth(3.0);
        glColor3f(1.0,0.0,0.0);
        glBegin(GL_LINES);
            glVertex3d(waypointTail->waypoint[X],waypointTail->waypoint[Y],0.0);
            glVertex3d(cursorLocation[X],cursorLocation[Y],0.0);
        glEnd();
    }
*/
    if (pathNeedsFullUpdate) //this is for direct interaction, such as ValueDrag
    {
        updateCorridor();
        updatePath(end); //required because of an unknown bug in updateCorridor
    }

    if (selectedFlag)
    {
        pointHandle->setDrawMode(PlanarPointHandle::DrawSimpleArrows);
        for (idx = 1, cursor = (WaypointNode *)startIteration(); cursor != NULL; cursor = (WaypointNode *)next(), idx++)
        {
            pos_now = cursor->lineNode->position;
            if (renderingForPicking)
                glLoadName(idx);
            pointHandle->drawIn(win, pos_now, (idx == selectedNodeIndex)? 1 : 2,true);
        }
    }
    if (renderingForPicking)
    {
        glLoadName(0);
        //draw the corridor bounds for changing corridor width
        glPushName(0);
        for (idx = 1, cursor = (WaypointNode *)startIteration(); cursor != NULL; cursor = c_next, idx++)
        {
            c_next = (WaypointNode *)next();
            if (!c_next)
                break;

            glLoadName((int)(cursor->heading - 90));
            pos_now = cursor->port;
            pos_next = c_next->port;
            glBegin(GL_LINES);
                glVertex3f(pos_now[X], pos_now[Y], zlevel);
                glVertex3f(pos_next[X], pos_next[Y], zlevel);
            glEnd();

            glLoadName((int)(cursor->heading + 90));
            pos_now = cursor->starboard;
            pos_next = c_next->starboard;
            glBegin(GL_LINES);
                glVertex3f(pos_now[X], pos_now[Y], zlevel);
                glVertex3f(pos_next[X], pos_next[Y], zlevel);
            glEnd();
        }
        glPopName();

        if (selectedFlag)
            return;
    }

    //remember: Translate by your local offset before rendering
    if (vessel)
    {
        // current pos (represents where the boat is now)
        vessel->getPosable()->setPosition(predictedPosition[X],predictedPosition[Y],zlevel);


        // need to rotate bow to line up with heading value on planned path. Or RSA from file.
        vessel->setCourse(currentHeading,true);

    }
    else
    {
        glColor3f(1.0,1.0,0.0);
        glPointSize(15.0);

        glBegin(GL_POINTS);
        glVertex3d(predictedPosition[X],predictedPosition[Y],zlevel);
        glEnd();
    }

    glDisable(GL_LIGHTING);
    //centerline
    if (start != end && (!selectedFlag || !renderingForPicking))
    {
        glLineWidth(3.0);
        glBegin(GL_LINE_STRIP);
        for (idx = 1, cursor = (WaypointNode *)startIteration(); cursor != NULL; cursor = c_next, idx++)
        {
            t_now = getTime();
            pos_now = cursor->lineNode->position;

            if(currentGzTime > t_now)
                glColor3f(0.5,0.5,0.5);
            else
                glColor3f(0.1f,0.7f,0.1f);

            glVertex3f(pos_now[X], pos_now[Y],zlevel);

            c_next = (WaypointNode *)next();
            if (!c_next)
                break;
            t_next = getTime();
            pos_next = c_next->lineNode->position;
            if(c_next && currentGzTime > t_now && currentGzTime < t_next)
            {
                t = (currentGzTime - t_now) / (t_next - t_now);
                midx = (float)(pos_now[X] + (t * (pos_next[X] - pos_now[X])));
                midy = (float)(pos_now[Y] + (t * (pos_next[Y] - pos_now[Y])));

                glColor3f(0.5,0.5,0.5);
                glVertex3f(midx, midy, zlevel);

                glColor3f(0.0,1.0,0.0);
                glVertex3f(midx, midy, zlevel + 0.1f);
            }
        }
        glEnd();
    }

    if (start != end)
    {
        // port limit line
        glColor3f(1.0,0.0,1.0);
        glBegin(GL_LINE_STRIP);
        for (idx = 1, cursor = (WaypointNode *)startIteration(); cursor != NULL; cursor = c_next, idx++)
        {
            t_now = getTime();
            pos_now = cursor->port;

            if(currentGzTime > t_now)
                glColor3f(0.5,0.5,0.5);
            else
                glColor3f(0.6f,0.6f,0.2f);

            glVertex3f(pos_now[X], pos_now[Y], zlevel);

            c_next = (WaypointNode *)next();
            if (!c_next)
                break;
            t_next = getTime();
            pos_next = c_next->port;
            if(c_next && currentGzTime > t_now && currentGzTime < t_next)
            {
                t = (currentGzTime - t_now) / (t_next - t_now);
                midx = (float)(pos_now[X] + (t * (pos_next[X] - pos_now[X])));
                midy = (float)(pos_now[Y] + (t * (pos_next[Y] - pos_now[Y])));

                glColor3f(0.5,0.5,0.5);
                glVertex3f(midx, midy, zlevel);

                glColor3f(1.0,0.0,1.0);
                glVertex3f(midx, midy, zlevel);
            }
        }
        glEnd();

        // starboard limit line
        glColor3f(1.0,0.0,1.0);
        glBegin(GL_LINE_STRIP);
        for (idx = 1, cursor = (WaypointNode *)startIteration(); cursor != NULL; cursor = c_next, idx++)
        {
            t_now = getTime();
            pos_now = cursor->starboard;

            if(currentGzTime > t_now)
                glColor3f(0.5,0.5,0.5);
            else
                glColor3f(0.6f,0.6f,0.2f);

            glVertex3f(pos_now[X], pos_now[Y], zlevel);

            c_next = (WaypointNode *)next();
            if (!c_next)
                break;
            t_next = getTime();
            pos_next = c_next->starboard;
            if(c_next && currentGzTime > t_now && currentGzTime < t_next)
            {
                t = (currentGzTime - t_now) / (t_next - t_now);
                midx = (float)(pos_now[X] + (t * (pos_next[X] - pos_now[X])));
                midy = (float)(pos_now[Y] + (t * (pos_next[Y] - pos_now[Y])));

                glColor3f(0.5,0.5,0.5);
                glVertex3f(midx, midy, zlevel);

                glColor3f(1.0,0.0,1.0);
                glVertex3f(midx, midy, zlevel);
            }
        }
        glEnd();
    }
    if (tclTableNeedsUpdating)
    {
        tclTableNeedsUpdating = GL_FALSE;
        fillTclArray();
    }

    for (idx = 1, cursor = (WaypointNode *)startIteration(); cursor != NULL; cursor = (WaypointNode *)next(), idx++)
    {
        pos = cursor->lineNode->position;
        //print the label
        std::string l;
        if (!cursor->label.empty())
            l = getWaypointLabel(*cursor);
        else
            l = boost::lexical_cast<std::string>(idx);
        glColor4f(1.0, 1.0, 1.0, 0.7f);
        //glLoadIdentity();
        win->printOver(pos[X], pos[Y], pos[Z] + 3.0, l, GL_TRUE, GLUT_BITMAP_HELVETICA_12, 5, 5);
    }
/*
    //Draw labels near cursor
    p = win->getPoseInfo();
    scale = .0001f / p->getScale();
    for (idx = 1, cursor = (WaypointNode *)startIteration(); cursor != NULL; cursor = (WaypointNode *)next(), idx++)
    {
        pos = cursor->lineNode->position;
        if (win->getCursorPosOnPlane(x, y, pos[Z])) //see if the cursor is nearby.  If so, show label.
        {
            x -= pos[X];
            y -= pos[Y];
            if ((x * x + y * y) < (900.0 * scale * scale)) // are we within 30m(?) of the waypoint?
            {
                glColor4f(1.0, 1.0, 1.0, 0.7f);
                glLoadIdentity();
                //if (renderMode == Glyph::Render2D_atScreen)
                    win->printOver(pos[X], pos[Y], pos[Z] + 3.0, getWaypointLabel(cursor));
            //  else
                    //win->printAt(pos[X], pos[Y], pos[Z] + 3.0, getWaypointLabel(cursor));
                    
            }
        }
    }
*/

    glPopAttrib();
    //this will draw the vessel on the path
    ObjectCollection::DrawLowRes(degradation,win);

}

/** Loads (by looping) textures, objects, collections, scripts and animators from an XML document.
 * @param[in] fname the filename of where other objects reside (like a vessel) must be const!
 * \return Object* - A pointer to the object loaded
 * \par Purpose: 
 * in this case it will load any objects associated with the navigation path. (like the vessel)
 * \note Needed a way to capture the object pointer in order to update things correctly when rendered.  
 * \sa LoadXML(char*)
 * \author briana sullivan (bs).
 * \date 5/17/2005
 */
Object * NavigationPath::LoadXML(const char * fname)
{
    return ObjectCollection::LoadXML(fname);
}

/** Loads Waypoints and such.
 * @param[in] doc a pointer to an XML formatted document.
 * @param[in] cur the current item that is being evaluated.
 * @param[in] path of the directory from which the XML file is loaded.
 * \return Object* - this object
 * \author Matt Plumlee (mdp)
 * \date 2/12/2007
 */
Object * NavigationPath::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
//  LogState ls(&gzlog,"NavigationPath::LoadXML",TRACE);
    double t=0.0, pos[3];
    WaypointNode *newnode;
    int len;
    std::string name;
    const double defaultTimeElapsed = 60.0;

    if (!Object::LoadXML(xml, path, ncfile))
        return NULL;

    corridorWidth = xml.get("<xmlattr>.corridorWidth",corridorWidth);
    defaultSpeed = xml.get("<xmlattr>.defaultSpeed",defaultSpeed);
    zlevel = xml.get("<xmlattr>.zLevel",zlevel);
    dayCostRate = xml.get("<xmlattr>.dayCostRate",dayCostRate);
    fuelCostRate = xml.get("<xmlattr>.fuelCostRate",fuelCostRate);
    t = xml.get("<xmlattr>.startTime",t);

    for(bpt::ptree::const_iterator node = xml.begin(); node != xml.end(); ++node)
        if(node->first == "Waypoint")
        {
            newnode = new WaypointNode;
            
            boost::optional<std::string> attribute;
            if(attribute = node->second.get_optional<std::string>("<xmlattr>.position"))
            {
                std::istringstream ss(*attribute);
                ss >> pos[0];
                ss >> pos[1];
                ss >> pos[2];
            }

            newnode->speed = node->second.get("<xmlattr>.speed",newnode->speed);
            insert(t, newnode, pos[X], pos[Y], pos[Z], true);

            if(!name.empty())
                newnode->label = name;

            t = end->time + defaultTimeElapsed;
        }

    return this;
}

/*---------------------------------------------------------------------------*/
/** \author mdp
 *  \date 2/9/2007
 */
void NavigationPath::SaveXML(FILE * out, std::string const &path)
{
    TimeSequenceNode *current;
    TimeLineNode *node;

    fprintf(out,"<Object type=\"NavigationPath\" ");
/*
    if(GetFilename())
        fprintf(out,"filename=\"%s\" ",GetFilename().getFilenameFromPath(path).c_str());
*/
    SaveXMLattributes(out);
    fprintf(out,">\n");

    for (current = start; current; current = current->next)
    {
        node = (TimeLineNode*) current->data;
        SaveXMLnodeAttributes(out, node);
        fprintf(out,"/>\n");
    }
    fprintf(out,"</Object>\n");
}

/*---------------------------------------------------------------------------*/
/** \author mdp
 *  \date 2/9/2007
 */
void NavigationPath::SaveXMLattributes(FILE * out)
{
    ObjectCollection::SaveXMLattributes(out);
    fprintf(out,"corridorWidth=\"%f\" ",corridorWidth);
    fprintf(out,"defaultSpeed=\"%f\" ",defaultSpeed);
    fprintf(out,"zLevel=\"%f\" ",zlevel);
    fprintf(out,"dayCostRate=\"%f\" ",dayCostRate);
    fprintf(out,"fuelCostRate=\"%f\" ",fuelCostRate);
    fprintf(out,"startTime=\"%f\" ", start->time);
}

/*---------------------------------------------------------------------------*/
/** \author mdp
 *  \date 2/12/2007
 */
void NavigationPath::SaveXMLnodeAttributes(FILE * out, TimeLineNode *node)
{
    WaypointNode *wn = (WaypointNode *)node;

    gz4d::Point<double> fa = wn->lineNode->position;
    fprintf(out,"<Waypoint position=\"%f %f %f\" ", fa[0], fa[1], fa[2]);
    fprintf(out,"speed=\"%f\" ", wn->speed);
    if (!wn->label.empty())
        fprintf(out,"label=\"%s\" ", wn->label.c_str());
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
WaypointNode *NavigationPath::insert(double t, double x, double y, double z)
{
    WaypointNode *newnode = new WaypointNode;
    insert(t, newnode, x, y, z);
    return newnode;
}

/*---------------------------------------------------------------------------*/
void NavigationPath::insert(double t, WaypointNode *waypoint, double x, double y, double z, bool loading)
{
    TimeSequenceNode *newNode;
    //WaypointNode *cursor;

    if (!loading)
    {
        waypoint->speed = defaultSpeed;
    }
    waypoint->eta = 0;
    waypoint->cost = 0;
    waypoint->row = rowCount;

    //working on this to be able to insert new waypoint at appropriate place in the path planning table
    if(indexedNode)
    {
        //curNode = start;
        //for(cursor = (WaypointNode *) startIteration(); cursor != NULL; cursor = (WaypointNode *)next())
            //cout<<cursor->row<<endl;

        //cout<<endl;

    }

    //insert row in table must go before timeline insert
    if(!insertRow(mainWindow->interp,waypoint))
        cerr<<"tcl evaluation failed: NavigationPath::insert()"<<endl;


    //this inserts the new waypoint into the timeline
    newNode = TimeLine::insert(t, waypoint, x, y, z);
    zlevel = z;

    bbox.expand(waypoint->lineNode->position);
    bbox.expandTime(t);


    //links the newly updated timeLine (navigation path) with the tcl table
    //(the path planning table.) Must go after row and timeline insertion.
    updatePath(newNode);


}

/*---------------------------------------------------------------------------*/
/** fills passed waypoint structure with interpolated info for given time.
 */
bool NavigationPath::getWaypoint(double t, WaypointNode *waypoint)
{
    double weight;
    gz4d::Point<double> position;
    WaypointNode *info1, *info2;

    info1 = getWaypoint(t, weight, info2, waypoint->lineNode->position);
    if (info1)
    {
        position = waypoint->lineNode->position;
        waypoint->cos_h = info1->cos_h;
        waypoint->sin_h = info1->sin_h;
        waypoint->distanceToNext = (1 - weight) * info1->distanceToNext;
        waypoint->heading = info1->heading;
        waypoint->speed = info1->speed;
        waypoint->port[X] = position[X];
        waypoint->port[Y] = position[Y];
        waypoint->starboard[X] = position[X];
        waypoint->starboard[Y] = position[Y];
        return true;
    }
    return false;
}

/*---------------------------------------------------------------------------*/
/** returns waypoint info at time t with support information to make
 *  interpolations on other data actually in the derived WaypointNode data type.
 */
WaypointNode *NavigationPath::getWaypoint(double t, double &weight, WaypointNode *&info2, gz4d::Point<double> &position)
{
    WaypointNode *info1;
    TimeLineNode *v;

    info1 = (WaypointNode*)getPosition(t, weight, v, position);
    if (!info1)
        return NULL;
    info2 = (WaypointNode*) v;

    return info1;
}

/*---------------------------------------------------------------------------*/
void NavigationPath::updateEntirePath()
{
    TimeSequenceNode *current = end;

    while (current)
    {
        updatePath(current);
        current = current->prev;
    }
}

/*---------------------------------------------------------------------------*/
void NavigationPath::updatePath(TimeSequenceNode *becauseOfMe)
{
    TimeSequenceNode *prev, *next, *current, *tempCurrent, *tempPrev;
    WaypointNode *prevWaypoint = NULL, *nextWaypoint, *currentWaypoint, *tempCurWaypoint, *tempPrevWaypoint;
    gz4d::Point<double> pos1, pos2;
    float h, dx, dy, dtn, s, ttg;

    current = becauseOfMe;
    if (!current)
        return;
    prev = current->prev;
    next = current->next;

    currentWaypoint = (WaypointNode*) current->data;
    if (!initializedWithTcl && current)
    {
        tellTclWeExist(mainWindow->interp);
        initializedWithTcl = GL_TRUE;
    }

    //calculate distance, heading and time values
    if (prev)
    {
        prevWaypoint = (WaypointNode*) prev->data;
        pos1 = prevWaypoint->lineNode->position;
        pos2 = currentWaypoint->lineNode->position;
        dx = pos2[X] - pos1[X];
        dy = pos2[Y] - pos1[Y];
        //in meters, * 3.28
        prevWaypoint->distanceToNext = dtn = sqrt(dx * dx + dy * dy);
        h = atan2(dy, dx);
        prevWaypoint->heading = h * 180 / M_PI;
        prevWaypoint->cos_h = (float)cos(h);
        prevWaypoint->sin_h = (float)sin(h);
        s = prevWaypoint->speed;
        prevWaypoint->timeToGo = ttg = dtn/s;
        prevWaypoint->cost = ttg * dayCostRate + ttg * s * s * fuelCostRate; //assume cost goes up linearly with time, and with the square of your speed in a given slice of time.
        if(currentWaypoint->timeToGo < 0)
            currentWaypoint->timeToGo = 0;
        currentWaypoint->eta = prevWaypoint->eta + prevWaypoint->timeToGo;
        current->time = start->time + currentWaypoint->eta;
    }
    if (next)
    {
        nextWaypoint = (WaypointNode*) next->data;
        pos1 = nextWaypoint->lineNode->position;
        pos2 = currentWaypoint->lineNode->position;
        dx = pos1[X] - pos2[X];
        dy = pos1[Y] - pos2[Y];
        //in meters, * 3.28
        currentWaypoint->distanceToNext = dtn = sqrt(dx * dx + dy * dy);
        h = atan2(dy, dx);
        currentWaypoint->heading = h * 180 / M_PI;
        currentWaypoint->cos_h = (float)cos(h);
        currentWaypoint->sin_h = (float)sin(h);
        s = currentWaypoint->speed;
        currentWaypoint->timeToGo = ttg = dtn/s;
        currentWaypoint->cost = ttg * dayCostRate + ttg * s * s * fuelCostRate;

        for (tempCurrent = becauseOfMe; tempCurrent != NULL; tempCurrent = tempCurrent->next)
        {
            tempCurWaypoint = (WaypointNode*) tempCurrent->data;
            tempPrev = tempCurrent->prev;
            if(tempPrev)
            {
                tempPrevWaypoint = (WaypointNode*) tempPrev->data;
                tempCurWaypoint->eta = tempPrevWaypoint->timeToGo + tempPrevWaypoint->eta;
                tempCurrent->time = start->time + tempCurWaypoint->eta;
            }
        }
    }
    else
    {
        if (prev)
            currentWaypoint->heading = prevWaypoint->heading;
        else
        {
            currentWaypoint->heading = 0;
            currentWaypoint->timeToGo = 0;
        }
        currentWaypoint->distanceToNext = 0;
    }

    updateCorridor(prev, next);

    //calculate total distance and cost
    totalDistance = 0;
    totalCost = 0;
    etaCumulative = 0;
    for (current = start; current != NULL; current = current->next)
    {
        currentWaypoint = (WaypointNode*) current->data;

        if (current->next)
            totalDistance += currentWaypoint->distanceToNext;
        totalCost += currentWaypoint->cost;
        etaCumulative += currentWaypoint->timeToGo;
        if(current == start)
            currentWaypoint->eta = 0;
    }

    pathNeedsFullUpdate = false;
    if (initializedWithTcl && !deletingPoint)
        tclTableNeedsUpdating = GL_TRUE;
//  globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*---------------------------------------------------------------------------*/
void NavigationPath::updateCorridor(TimeSequenceNode *startAt, TimeSequenceNode *endAt)
{
    float differenceAngle, splitAngle, dist, xOffset, yOffset;
    TimeSequenceNode *prev, *current;
    WaypointNode *prevWaypoint, *currentWaypoint;
    gz4d::Point<double> pos;

    if (!startAt)
        startAt = start;
    if (!endAt)
        endAt = end;

    for (current = startAt; current != NULL; current = current->next)
    {
        prev = current->prev;

        currentWaypoint = (WaypointNode *) current->data;
        pos = currentWaypoint->lineNode->position;
        //calculate location for port and starboard lines
        if (prev)
        {
            prevWaypoint = (WaypointNode *) prev->data;
            differenceAngle = (currentWaypoint->heading + (prevWaypoint->heading + 180)) / 2.0;
            splitAngle = differenceAngle - (prevWaypoint->heading + 180);
        }
        else
        {
            differenceAngle = currentWaypoint->heading + 90;
            splitAngle = -90;
        }
        splitAngle *= (float)(M_PI / 180.0);
        differenceAngle *= (float)(M_PI / 180.0);
        dist = (float) (corridorWidth / sin(splitAngle));
        xOffset = dist * (float)cos(differenceAngle);
        yOffset = dist * (float)sin(differenceAngle);
        currentWaypoint->port[X] = pos[X] - xOffset;
        currentWaypoint->port[Y] = pos[Y] - yOffset;
        currentWaypoint->starboard[X] = pos[X] + xOffset;
        currentWaypoint->starboard[Y] = pos[Y] + yOffset;

        if (current == endAt) //exit loop
            break;
    }
    updateCounter++;
    if (updateCounter < 0)
        updateCounter = 0;
}

/*---------------------------------------------------------------------------*/
void NavigationPath::updateTimes(double *plannedPathTime, const double *pos,
                                 float cellsize, int maxx, int maxy,
                                 int &navPathUpdateCounter)
{
    int  x, y, idx;
    gz4d::Point<double> p;

    //first, see if anything has changed since the last request (if not, return)
    if (navPathUpdateCounter == updateCounter)
        return;
        
    p[Z] = 0;
    for (x = 0; x < maxx; x++)
    {
        p[X] = x * cellsize;
        for (y = 0; y < maxy; y++)
        {
            p[Y] = y * cellsize;
            idx = y * maxx + x;
            plannedPathTime[idx] = getTimeAt(p);
        }
    }
    /*
    if (!start)
        return NAN;

    for (current = start; current->next != NULL; current = current->next)
    {
        currentWaypoint = (WaypointNode *)current->data;
        if (!currentWaypoint)
            continue;

        if (current == start)
        {
            port1 = currentWaypoint->port;
            startboard1 = currentWaypoint->starboard;
            t1 = current->time;
            continue;
        }

        port2 = currentWaypoint->port;
        startboard2 = currentWaypoint->starboard;
        t2 = current->time;

        //return t + (current->next->time - t) * (newY / len);
    }
    */

    navPathUpdateCounter = updateCounter;
}

/*---------------------------------------------------------------------------*/
double NavigationPath::getTimeAt(gz4d::Point<double> const &p)
{
    double t;
    TimeSequenceNode *current;
    WaypointNode *currentWaypoint;
    double newX, newY;
    double px, py, len;
    gz4d::Point<double> pos;

    if (!start)
        return gz4d::Nan<double>();

    for (current = start; current->next != NULL; current = current->next)
    {
        currentWaypoint = (WaypointNode *)current->data;
        if (!currentWaypoint)
            continue;

        //rotate point around the waypoint so we can treat line as always being vertical in Y.
        //(we do this 'backwards' because heading is along the Y axis and heads toward the X rather than vice versa)
        pos = currentWaypoint->lineNode->position;
        px = p[X] - pos[X];
        py = p[Y] - pos[Y];
        newX = py * currentWaypoint->cos_h - px * currentWaypoint->sin_h;
        newY = py * currentWaypoint->sin_h + px * currentWaypoint->cos_h;

        if (newX > corridorWidth || newX < -corridorWidth) //outside the corridor
            continue;
//&&&Ok, so here we really need to check against where our outer lines are...
        len = currentWaypoint->distanceToNext;
        if (newY < -corridorWidth || newY >= len) //off one end or the other (we give it a little slop on the back end to help at joints).
            continue;

        //if we got here, we've found a viable path segment.
        t = current->time;
        if (newY <= 0)
            return t; //just assume that time is the starting time for this segment.

        return t + (current->next->time - t) * (newY / len);
    }
    return gz4d::Nan<double>();
}

/*---------------------------------------------------------------------------*/
void NavigationPath::Idle(double time)
{
    Object::Idle(time);

    double lasttime = currentGzTime, t1, t2, tFactor;
    gz4d::Point<double> pos, pos2;
    currentGzTime = time;
    if(time == lasttime)
        return;

    // calculate current pos
    if(start)
    {
        if (time <= bbox.start)
        {
            pos = ((WaypointNode*)(start->data))->lineNode->position;
            predictedPosition[X] = pos[X];
            predictedPosition[Y] = pos[Y];
            currentHeading = 90 - ((WaypointNode*)(start->data))->heading;
        }
        else if (time >= bbox.end)
        {
            pos = ((WaypointNode*)(end->data))->lineNode->position;
            predictedPosition[X] = pos[X];
            predictedPosition[Y] = pos[Y];
            //currentHeading = ((WaypointNode*)(end->data))->heading;
        }
        else
        {
            while(mostRecentNode && mostRecentNode->time >= time)
                mostRecentNode = mostRecentNode->prev;
            if (!mostRecentNode)
                mostRecentNode = start;

            while(mostRecentNode && mostRecentNode->time < time)
                mostRecentNode = mostRecentNode->next;
            pos = ((WaypointNode*)(mostRecentNode->data))->lineNode->position;
            pos2 = ((WaypointNode*)(mostRecentNode->prev->data))->lineNode->position;
            currentHeading = 90 - ((WaypointNode*)(mostRecentNode->prev->data))->heading;
            t1 = mostRecentNode->time;
            t2 = mostRecentNode->prev->time;
            tFactor = ((time - t2) / (t1 - t2));
            predictedPosition[X] = pos2[X] + ((pos[X] - pos2[X]) * tFactor);
            predictedPosition[Y] = pos2[Y] + ((pos[Y] - pos2[Y]) * tFactor);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
        }
    }
}

/*---------------------------------------------------------------------------*/
int NavigationPath::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
    int numNames = names[0];

    if (pickType == PICK_CLICK1 && !selectedFlag)
    {
        setSelectedObject(this);
        if (isEditableFlag && (selectedNodeIndex != (int) names[1]))
        {
            selectedNodeIndex = names[1];
            selectedNode = indexNodeAt(selectedNodeIndex);
        }
        //ShowInfo(&names[1], numNames);
        highLevelEventMediator->Trigger(objectSelectedInputType->value, pose, (void*)selectedNodeIndex);
        return GL_TRUE;
    }
    else
    {
        if (pickType == PICK_PRESS1 && isEditableFlag)
        {
            if (((!selectedFlag && numNames >= 2) && (int) names[1] == 0) || ((selectedFlag && numNames >= 3) && (int) names[2] == 0))
            {
                globalInputMediator->PushInteractor(valueDrag);
                if (selectedFlag)
                    valueDrag->setAxisAngle((int) names[3]);
                else
                    valueDrag->setAxisAngle((int) names[2]);
                valueDrag->HandleInput(inputInfo->type, inputInfo);
                return GL_TRUE;
            }
            else if (numNames >= 2 && (selectedNodeIndex != (int) names[2]))
            {
                selectedNodeIndex = names[2];
                selectedNode = indexNodeAt(selectedNodeIndex);
            }
            //ShowInfo(&names[1], numNames);
            highLevelEventMediator->Trigger(objectSelectedInputType->value, pose, (void*)selectedNodeIndex);
        }

        return Object::pickAction(pickType, inputInfo, names, hits);
    }
}

/*---------------------------------------------------------------------------*/
void NavigationPath::wrapperChangedValue(PosableStateWrapper *wrapper, long &changeMask)
{
    TimeLine::wrapperChangedValue(wrapper, changeMask);

    if (changeMask && ChangeTranslateMask)
    {
        updatePath(indexedNode);
    }
}

/*---------------------------------------------------------------------------*/
/** returns GL_TRUE if the value could be inserted, GL_FALSE otherwise.
 */
GLboolean NavigationPath::insertValue(PosableStateWrapper *wrapper,
                                      GLboolean beforeCurrent, int dim)
{
    double t1, t2, newT;
    gz4d::Point<double> p1, p2;
    double defaultTimeElapsed = 60.0;
    float newX, newY, newSpeed, factor = 0.5f;
    TimeSequenceNode *n1, *n2;
    WaypointNode *w1, *w2;

///\bug still working on insertion, here... isn't quite right but good enough for now.
    if (indexNodeAt(wrapper->getIntegerIndex()))
    {
        if (beforeCurrent)
        {
            n2 = indexedNode;
            t2 = n2->time;
            n1 = n2->prev;
            if (n1)
            {
                t1 = n2->time;
                newT = (t1 + t2) / 2.0;
            }
            else
            {
                if (n2->next)
                {
                    n1 = n2->next;
                    t1 = t2 + t2 - n2->next->time; //project backwards
                    factor = 2;
                }
                else
                {
                    t1 = t2 - defaultTimeElapsed; //just pick somethin'
                    factor = 0;
                    n1 = n2; //ok, set our node to base things off of to n1.
                }
                newT = t1;
            }
        }
        else
        {
            n1 = indexedNode;
            t1 = n1->time;
            n2 = n1->next;
            if (n2)
            {
                t2 = n2->time;
                newT = (t1 + t2) / 2.0;
            }
            else
            {
                if (n1->prev)
                {
                    n2 = n1->prev;
                    t2 = t1 + t1 - n1->prev->time; //project forwards
                    factor = -1;
                }
                else
                {
                    t2 = t1 + defaultTimeElapsed; //just pick something
                    factor = 0;
                    n2 = n1; //set our node to base things off of to n2.
                }
                newT = t2;
            }
            selectedNodeIndex = nodeIndex;
            selectedNode = indexNodeAt(selectedNodeIndex);
        }
        w1 = ((WaypointNode *)n1->data);
        w2 = ((WaypointNode *)n2->data);
        p1 = w1->lineNode->position;
        p2 = w2->lineNode->position;
        if (factor < 0)
            newSpeed = w2->speed;
        else
            newSpeed = w1->speed;
        if (factor == 0)
        {
            newX = p1[X] + newSpeed * defaultTimeElapsed / 1.414f;
            newY = p1[Y] + newSpeed * defaultTimeElapsed / 1.414f;
        }
        else
        {
            //don't have to worry about calculating the speed, because we're using the same speed through the new point...
            newX = p1[X] + factor * (p2[X] - p1[X]);
            newY = p1[Y] + factor * (p2[Y] - p1[Y]);
        }
        insert(newT, newX, newY, zlevel);
        resetIndexedNode();
        return GL_TRUE;
    }
    return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/** returns GL_TRUE if the value could be deleted, GL_FALSE otherwise.
 */
GLboolean NavigationPath::deleteValue(PosableStateWrapper *wrapper,
                                      int dim)
{
    WaypointNode *w;
    TimeSequenceNode *upd;

    if (indexNodeAt(wrapper->getIntegerIndex()))
    {
        w = (WaypointNode *)indexedNode->data;
        upd = indexedNode->next;
        if (!upd)
            upd = indexedNode->prev;
        if (!upd)
            return GL_FALSE; //can't remove last point!
        deletingPoint = GL_TRUE;
        removeFromLine(indexedNode);
        remove(indexedNode);
        
        updatePath(upd);
        resetIndexedNode();
        deletingPoint = GL_FALSE;
        fillTclArray();
        //delete row in table
        if(!deleteRow(mainWindow->interp,w->row))
            cerr<<"tcl evaluation failed: NavigationPath::deleteValue()"<<endl;
        
        delete w;
    }
    return GL_FALSE;
}

/*-------------------------Functions to Interface w/ Tcl---------------------*/
// perform, list, tellTclWeExist, fillTclArry, updateFromTclTable
// insertRow, deleteRow
/*---------------------------------------------------------------------------*/
int NavigationPath::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    char retString[20];
    const char *message = argv[0];
    
    if (argc >= 1)
    {
        if (!strcmp(message, "connectToTable"))
        {
            tellTclWeExist(interp);
            return TCL_OK;
        }
        if (!strcmp(message, "updateFromTable"))
        {
            updateFromTclTable(interp, argv[1], argv[2]);
            return TCL_OK;
        }
        if (!strcmp(message, "refresh"))
        {
            updateEntirePath();
            fillTclArray(interp);
            return TCL_OK;
        }
        if (!strcmp(message, "dayRate"))
        {
            if (argc >=2)
            {
                dayCostRate = (float)atof(argv[1]);
                updateEntirePath();
            }
            sprintf(retString, "%6.4f", dayCostRate);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "fuelRate"))
        {
            if (argc >=2)
            {
                fuelCostRate = (float)atof(argv[1]);
                updateEntirePath();
            }
            sprintf(retString, "%6.4f", dayCostRate);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
		if (!strcmp(message, "startTime"))
		{
			if (argc >= 2)
			{
				start->time = atof(argv[1]);
				updateEntirePath();
			}
			sprintf(retString, "%f", start->time);
			Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
			return TCL_OK;
		}
        /** corridor used to dynamically change the corridor width of the navpath
          * (both the port and stern lines).
          * \note no argument will return current corridor width
          * \author Briana Sullivan (bs)
          * \date 06/30/05
          */
        if (!strcmp(message, "corridor"))
        {
            if (argc >= 2)
            {
                corridorWidth = atof(argv[1]);
                updateCorridor();
                updatePath(end);
                //updatePath(start);
                //updatePath(indexedNode);
                // if you want to update only after the point you picked use this
                //globalInputMediator->Trigger(Scene3D::objectChangedEvent()); 
            }
            //Tcl_AppendResult(interp, "usage: corridor [width]", NULL);
            sprintf(retString, "%.2f", corridorWidth);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "save"))
        {

            
        }
    }
    return Object::perform(argc, argv, interp);

}

/*---------------------------------------------------------------------------*/
int NavigationPath::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "connectToTable updateFromTable refresh dayRate fuelRate startTime corridor ", NULL);
    return Object::list(interp);
}

/*---------------------------------------------------------------------------*/
GLboolean NavigationPath::tellTclWeExist(Tcl_Interp *interp)
{
    string pathName = getAddress()->getAddress();
    string varName = getAddress()->getName();
    char *command;
    
    if (!local_interp)
        local_interp = interp;

    command = new char[20 + pathName.size() + varName.size()];

    sprintf(command, "newNavPath %s %s", pathName.c_str(), varName.c_str());

    if (Tcl_Eval(interp, command) != TCL_OK)
    {
        cerr << "Unable to create NavPath window properly" << endl;
        return GL_FALSE;
    }

    fillTclArray(interp);

    Window3D::setBridgeView(false);
//  if (Tcl_Eval(interp, "panePack $pathPlanWindow 1") != TCL_OK)
//      return GL_FALSE;

    sprintf(command, ".pathPlanning.table insert rows 1 1; .pathPlanning.table tag row oddRow 1");
    if (Tcl_Eval(interp, command) != TCL_OK)
        return GL_FALSE;

    delete command;

    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
GLboolean NavigationPath::fillTclArray(Tcl_Interp *interp)
{
    int idx, idx2;
    WaypointNode *cursor;
    char indexText[10];
    char value[256];
    double lat, lon, x, y;
    gz4d::geo::Projection proj;
    char varName[256];
    float heading;
    Scene3D *scene3d = mainWindow->getScene3D();

    strcpy(varName, getAddress()->getName().c_str()); //need to copy since it is fickle c_str()
    if (!interp)
        interp = local_interp;

    proj = gz4d::geo::Projection(spatialReference,scene3d->getSpatialReference());

    for (idx = 1, cursor = (WaypointNode *)startIteration(); cursor != NULL; cursor = (WaypointNode *)next(), idx++)
    {
        //put something in Label column
        sprintf(indexText, "%d,%d", idx, 1);
        Tcl_SetVar2(interp, varName, indexText, cursor->label.c_str(), TCL_GLOBAL_ONLY);

        //put something in SMG column
        sprintf(value, "%.2f", cursor->speed * KNOTS_OVER_M_PER_S);
        sprintf(indexText, "%d,%d", idx, 2);
        Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);

        //put something in DTNW column
        sprintf(value, "%.2f", cursor->distanceToNext / METERS_OVER_MILES);
        sprintf(indexText, "%d,%d", idx, 3);
        Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);

        //put something in TTG column 
        formatSeconds(cursor->timeToGo, value);
        sprintf(indexText, "%d,%d", idx, 4);
        Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);

        //put something in Heading column
        heading = 90 - cursor->heading;
        if (heading < 0)
            heading += 360;
        sprintf(value, "%.2f", heading);
        sprintf(indexText, "%d,%d", idx, 5);
        Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);

        //put something in ETA column
        //TimePieces*tpETA = new TimePieces;
        //tpETA->decompose(time(0),Scene3D::timeControl->getTimeZone());
        //formatSeconds(cursor->eta,value);
        TimeControl::getTimeStr(value, cursor->eta + start->time);
        sprintf(indexText, "%d,%d", idx, 6);
        Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);

        //put something in Lat, Lon columns (in meters)
        y = cursor->lineNode->position[Y];
        x = cursor->lineNode->position[X];
        
        if (proj)
        {
            gz4d::geo::Position<double> p = proj.inverse(x, y);
            lon = p.longitude.value();
            lat = p.latitude.value();
        }
        else
        {
            lon = x;
            lat = y;
        }

        //format lat/lon here 00000'00.00\"N" for path planning table
        DecimalDegrees dd(lat,lon);

        //proj->formatDecimalDegrees(lat, value, 1);
        sprintf(indexText, "%d,%d", idx, 7);
        Tcl_SetVar2(interp, varName, indexText, dd.lat_string.c_str(), TCL_GLOBAL_ONLY);

        //proj->formatDecimalDegrees(lon, value, 0);
        sprintf(indexText, "%d,%d", idx, 8);
        Tcl_SetVar2(interp, varName, indexText, dd.lon_string.c_str(), TCL_GLOBAL_ONLY);

        //put something in cost column 
        sprintf(value, " $%8.2f", cursor->cost);
        sprintf(indexText, "%d,%d", idx, 9);
        Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);
    }
    for (idx2 = 1; idx2 <= 8; idx2++)
    {
        sprintf(indexText, "%d,%d", idx, idx2);
        Tcl_SetVar2(interp, varName, indexText, "", TCL_GLOBAL_ONLY);
    }
    //clear things that don't make sense
    sprintf(indexText, "%d,%d", idx - 1, 3);
    Tcl_SetVar2(interp, varName, indexText, "", TCL_GLOBAL_ONLY);
    sprintf(indexText, "%d,%d", idx - 1, 5);
    Tcl_SetVar2(interp, varName, indexText, "", TCL_GLOBAL_ONLY);

    //Fill in the bottom "total" row
    //put something in Label column
    sprintf(indexText, "%d,%d", idx, 1);
    Tcl_SetVar2(interp, varName, indexText, "TOTAL", TCL_GLOBAL_ONLY);
    //empty column
    sprintf(indexText, "%d,2", idx);
    Tcl_SetVar2(interp, varName, indexText, "", TCL_GLOBAL_ONLY);
    //total distance
    sprintf(value, "%.2f", totalDistance);
    sprintf(indexText, "%d,%d", idx, 3);
    Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);
    //Total Time
    formatSeconds(etaCumulative, value);
    sprintf(indexText, "%d,%d", idx, 4);
    Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);
    //Empty columns
    for (idx2 = 5; idx2 <= 8; idx2++)
    {
        sprintf(indexText, "%d,%d", idx, idx2);
        Tcl_SetVar2(interp, varName, indexText, "", TCL_GLOBAL_ONLY);
    }
    //total cost
    sprintf(value, " $%8.2f", totalCost);
    sprintf(indexText, "%d,%d", idx, 9);
    Tcl_SetVar2(interp, varName, indexText, value, TCL_GLOBAL_ONLY);
    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
GLboolean NavigationPath::updateFromTclTable(Tcl_Interp *interp,
                                             const char *varName,
                                             const char *index)
{
    int row, col, len;
    const char *c = index, *str;
    WaypointNode *cursor;
    double x, y, lat, lon;
    gz4d::geo::Projection proj;
    Scene3D *scene3d = mainWindow->getScene3D();

    if (!index || !varName)
        return GL_FALSE;

    row = atoi(c);
    while (*c != ',' && *c != '\0')
        c++;
    if (*c != ',') //bad index--no comma!
        return GL_FALSE;
    col = atoi(++c);

    cursor = (WaypointNode *)indexNodeAt(row);
    if (!cursor)
        return GL_FALSE;
    str = Tcl_GetVar2(interp, varName, index, TCL_GLOBAL_ONLY);
    if (!str)
        return GL_FALSE;

    switch (col)
    {
        case 1: //label
            len = strlen(str);
            if (len <= 0)
                break;
            cursor->label = str;
            break;
        case 2: //SMG
            Tcl_GetDouble(interp, str, &lon);
            cursor->speed = lon / KNOTS_OVER_M_PER_S;
            //updatePath(indexedNode);
            break;
        case 3: //DTNW
            Tcl_GetDouble(interp, str, &lon);
            cursor->distanceToNext = lon * METERS_OVER_MILES;
            //updatePath(indexedNode);
            break;
        case 5: //Heading
            Tcl_GetDouble(interp, str, &lon);
            if (lon >= 270)
                lon -= 360;
            cursor->heading = 90 - lon;
            //updatePath(indexedNode);
            break;
        case 7: //Lat
        case 8: //Lon
            proj = gz4d::geo::Projection(spatialReference,scene3d->getSpatialReference());
            y = cursor->lineNode->position[Y];
            x = cursor->lineNode->position[X];
            if (proj)
            {
                gz4d::geo::Position<double> p = proj.inverse(x, y);
                lon = p.longitude.value();
                lat = p.latitude.value();

                if (col == 7) //lat
                    Tcl_GetDouble(interp, str, &lat);
                else //lon
                    Tcl_GetDouble(interp, str, &lon);
                gz4d::Point<double> pp = proj(lat,lon);
                x = pp[0];
                y = pp[1];
            }
            else
            {
                if (col == 7) //lat
                    Tcl_GetDouble(interp, str, &y);
                else //lon
                    Tcl_GetDouble(interp, str, &x);
            }
            cursor->lineNode->position[Y] = y;
            cursor->lineNode->position[X] = x;
            //updatePath(indexedNode);
            break;
    }

    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
int NavigationPath::writeTo(FILE *file)
{
    /*
    int i, j, start, end;

    fprintf(file, "%s %d %d\n", Keyword(COLS_ROWS), maxx, maxy);
    fprintf(file, "%s %f\n", Keyword(CELLSIZE), cellsize);
    fprintf(file, "%s %s\n", Keyword(SHAPE), Keyword(shape));
    if (hasAlternateData)
        fprintf(file, "%s %s %f\n", Keyword(USE_ALTERNATE_DATA), Keyword(OFFSET), alternateDataOffset);
    Object::writeTo(file);

    start = 0;
    end = maxx;
    for (i = 0; i < maxy; i++)
    {
        if (shape != FULLGRID)
        {
            fprintf(file, "%d %d\n", startline[i], endline[i]);
            start = startline[i];
            end = endline[i];
        }
        for (j = start; j < end; j++)
        {
            if (hasAlternateData)
                fprintf(file, "%f %f\n", dem[i][j], alternateData[i][j] - alternateDataOffset);
            else
                fprintf(file, "%f\n", dem[i][j]);
        }
    }
*/
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
// documented in Object
int NavigationPath::readFrom(FILE *infile)
{
    /*
    float height;
    int i,j, retval;
    float minheight, maxheight;
//  double s = 0.0;
//  double e = 0.0;

    //parse header
    if (Object::readFrom(infile))
        return 1;

    
    createProgress(); //don't put it up until we know we're going to get here.
    if (!norms_array) //must not have been CONVERTed
    {
        retval = AllocateGrid();
        if (retval)
            return retval;

        minheight = 100000000.0;
        maxheight = -100000000.0;

    // timing stuff
//  mainWindow->getScene3D()->timeControl.updateCurrentTime();
//  s=mainWindow->getScene3D()->timeControl.getSysTime();
//  cerr << "Start: " << s << endl;

long filePos;

        if(binary)
        {
            filePos = ftell(in);
            fclose(in);
            in = fopen(GetFilename().string(), "rb");
            fseek(in,filePos,SEEK_SET);
        }

        
        if (shape == FULLGRID)
        {
            for (i = 0; i < maxy; i++)
            {
                if(binary)
                {
                    fread(dem[i],sizeof(float),maxx,in);
                    for (j = 0; j < maxx; j++)
                    {
                        height = dem[i][j];
                        if (height < 0)
                            height = -height; //want any holes to be properly accounted for when later restored.
                        if (height > 0)
                        {
                            if (height > maxheight) maxheight = height;
                            if (height < minheight) minheight = height;
                        }
                    }
                }
                else
                {
                    for (j = 0; j < maxx; j++)
                    {
                        fscanf(in, "%f", &height); //height = tmp;
                        dem[i][j] = height;
                        if (height < 0)
                            height = -height; //want any holes to be properly accounted for when later restored.
                        if (height > 0)
                        {
                            if (height > maxheight) maxheight = height;
                            if (height < minheight) minheight = height;
                        }
                    }
                }
                updateProgress(i*80/maxy);
            }
        }
        else
        {
            for (i = 0; i < maxy; i++)
            {
                if(binary)
                {
                    fread(&startline[i],sizeof(int),1,in);
                    fread(&endline[i],sizeof(int),1,in);
                    fread(&dem[i][startline[i]],sizeof(float),endline[i]-startline[i],in);
                    for (j = startline[i]; j < endline[i]; j++)
                    {
                        height = dem[i][j];
                        if (height < 0)
                            height = -height; //want any holes to be properly accounted for when later restored.
                        if (height > 0)
                        {
                            if (height > maxheight) maxheight = height;
                            if (height < minheight) minheight = height;
                        }
                    }
                }
                else
                {
                    fscanf(in,"%d %d",&startline[i],&endline[i]);
                    for (j = startline[i]; j < endline[i]; j++)
                    {
                        if (j >= maxx || i >= maxy)
                        {
                            cerr << "Bad row or column value in file (" << j << ", " << i << ")" << endl;
                            continue;
                        }
                        if (hasAlternateData)
                        {
                            fscanf(in, "%f %f", &height, &(alternateData[i][j]));
                            alternateData[i][j] += alternateDataOffset;
                        }
                        else
                            fscanf(in, "%f", &height);
                        dem[i][j] = height ;
                        if (height < 0)
                            height = -height; //want any holes to be properly accounted for when later restored.
                        if (height > 0)
                        {
                            if (height > maxheight) maxheight = height;
                            if (height < minheight) minheight = height;
                        }
                    }
                }
            updateProgress(i*80/maxy);
            }
        }
        // done reading file
        printf("\nDone loading.\n");
    }
    
    // timing stuff
//  mainWindow->getScene3D()->timeControl.updateCurrentTime();
//  e=mainWindow->getScene3D()->timeControl.getSysTime();
//  cerr << "End: " << e << endl;
//    cerr << "Elapsed: " << e-s << endl;

    bbox.w = 0;
    bbox.e = maxx*cellsize;
    bbox.s = 0;
    bbox.n = maxy*cellsize;
    if (minheight > 0.0)
        bbox.b = minheight;
    else
        bbox.b = 0.0;
    bbox.t = maxheight;

    //&&& part of the preferredOffset hack
    centerOfGutm[X] = (float) (bbox.e / 2.0f);
    centerOfGutm[Y] = (float) (bbox.e / 2.0f);
    centerOfGutm[Z] = GetHeightAt(centerOfGutm[X], centerOfGutm[Y]); //dem[maxy / 2][maxx / 2];
    if (centerOfGutm[Z] <= invalidHeight)
        centerOfGutm[Z] = (float) bbox.b;

    //cout << "Bottom  :" << bbox.b  << "\n";
    //cerr << "Gutm::bbox e " << bbox.e << " w " << bbox.w << " n " << bbox.n << " s " << bbox.s << endl;

    cout.flush();

    // calculate normals

    printf("Calculating normals...");
    updateTitle("Calculating normals...");

    if (normalsOnTheFly)
        CalculateNormals(1.0f);
    else
        CalculateNormals(lastHeightExaggeration);

    printf("Done.\n");

    killProgress();
    
    */
    // done
    return(0);
}



/*---------------------------------------------------------------------------
 * Update the row value of the waypoint depending on if it is the last waypoint
 * on the navigation path or not.
 * Also, propagate the new row values and eta values for all of the following 
 * waypoints on the path.
 * bms - added 3/31/05
 *----------------------------------------------------------------------------*/
GLboolean NavigationPath::insertRow(Tcl_Interp *interp, WaypointNode *waypoint)
{
    int row = waypoint->row;
    //const char *pathName = getAddress()->getAddress();
    //const char *varName = getAddress()->getName();

    char command[50];
    
    rowCount++;
    
    sprintf(command, ".pathPlanning.table insert rows %d 1", row);
    if (Tcl_Eval(interp, command) != TCL_OK)
    {
        cerr << "Unable to insert row" << endl;
        return GL_FALSE;
    }

    if (row & 1)
        sprintf(command, ".pathPlanning.table tag row oddRow %d", row);
    else
        sprintf(command, ".pathPlanning.table tag row evenRow %d", row);
    if (Tcl_Eval(interp, command) != TCL_OK)
    {
        cerr << "Unable to tag new row" << endl;
        return GL_FALSE;
    }

    sprintf(command, ".pathPlanning.table set %d,0 %d", row, row);
    if (Tcl_Eval(interp, command) != TCL_OK)
    {
        return GL_FALSE;
    }

    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
GLboolean NavigationPath::deleteRow(Tcl_Interp *interp, int row)
{
    rowCount--;

    char *command = new char[50];
    sprintf(command, ".pathPlanning.table delete rows %d 1", row);
    if(Tcl_Eval(interp, command) != TCL_OK)
    {
        delete [] command;
        return GL_FALSE;
    }
    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
std::string NavigationPath::getWaypointLabel(const WaypointNode& cursor)
{
    if(cursor.label.empty())
        return boost::lexical_cast<std::string>(cursor.row);
    return cursor.label;
}

/*---------------------------------------------------------------------------*/
void NavigationPath::formatSeconds(float secs, char* value)
{
    int hours, minutes, seconds;
    hours = (int)(secs/3600);
    minutes = ((int)secs - (hours*3600))/60;
    seconds = (int)secs - (hours*3600) - (minutes*60);
    sprintf(value,"  %03d:%02d:%02d",hours,minutes,seconds);
}


