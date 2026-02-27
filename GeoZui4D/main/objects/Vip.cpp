/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Vip.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  
 *  This object is used to visualize Ausi vips (auvs, instrument platforms, etc)
 *
 *  Created by rja June 13th, 2001
 */
/*---------------------------------------------------------------------------*/
#include "GZincludes.h"
#include <filesystem>
#include "engine/ObjectPoseState.h"
#include "Vip.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/Window3D.h"
#include "GeoData/Cone.h"
#include "GeoData/Cylinder.h"
#include "GeoData/DataLines.h"
#include "engine/ViewManager.h"
//#include "utilities/LinkedList.h"
#include <string.h>
#include <stdlib.h>
#include "Points.h"
#include "engine/InterpolationSupport.h"
#include "Target.h"
#include "engine/TimeControl.h"
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/geo/Projection.h>

extern int dominantEye;

extern MainWindow *mainWindow;

REGISTER_OBJECT(Vip, "vip");

static const char *VipNames[] = {OBJECT_NAMES, "NAME", "SPEED",
                                "VEHICLE", "INSTRUMENT", "PLATFORM",
                                "OPEN_SPACE_FRAME", "CUSTOM_SPACE_FRAME", "TORPEDO", "SOLAR_AUV", "CRAWLER", "MOORING", "ABE",
                                ""};
static Cone cone;
static Cylinder cylinder;


/*---------------------------------------------------------------------------*/
Vip::Vip()
: ObjectCollection(GL_FALSE,  PICK_CLICK1),
  tail(60, 0.1f)
{
    SetTypeAndVersion("VIP", 1, 0);
    SetKeywordTable(VipNames);
    pose = new ObjectPoseState(this);
    SetLabel("AUV");
    type = VEHICLE;
    subtype = TORPEDO;
//  vip_path = NULL;
//  currentBehavior = nextBehavior = NULL;
//  currentBehaviorIndex = nextBehaviorIndex = -1;
//  minSpeed = maxSpeed = 0.0f;

    dsl = NULL;

    first_time = true;
    width = length = height = 0.0;

    datafile = NULL;
    datatype = Unknown;
}

Vip::~Vip()
{

}

/*float Vip::getMinSpeed() const
{
    return minSpeed;
}

float Vip::getMaxSpeed() const
{
    return maxSpeed;
}

// haptics thread call
void Vip::detachCurrentBehavior()
{
    // update visual aspect of line
    if( vip_path && currentBehavior )
        vip_path->modifyLineAttrs(2.0, palette[WHITE], currentBehaviorIndex-1);

    //currentBehavior = nextBehavior = NULL;
    //currentBehaviorIndex = nextBehaviorIndex = -1;

    //cerr << "Detaching VIP behavior." << endl;
}

// haptics thread call
int Vip::attachCurrentBehavior(VIP_Behavior* beh)
{
    // iterate through behaviorList to find matching behavior
    if( beh != NULL )
    {
        currentBehavior = beh;
        currentBehaviorIndex = behaviorList.GetIndexOfFirstElement(beh);

        if( currentBehaviorIndex != -1 )
        {
            // update visual aspect of line
            vip_path->modifyLineAttrs(4.0, palette[YELLOW], currentBehaviorIndex-1);

            if( currentBehaviorIndex < behaviorList.Size() )
            {
                nextBehaviorIndex = currentBehaviorIndex+1;
                nextBehavior = (VIP_Behavior *)(behaviorList.GetElementAtIndex(nextBehaviorIndex));
            }
            else
            {
                nextBehaviorIndex = -1;
                nextBehavior = NULL;
            }
        }
        else
        {
            nextBehaviorIndex = -1;
            nextBehavior = NULL;
        }
    }
    else // NULL assumes we're at the VIP
    {
        currentBehavior = NULL;
        currentBehaviorIndex = -1;

        if( behaviorList.Size() > 0 ) // check for first behavior
        {
            nextBehaviorIndex = 1;
            nextBehavior = (VIP_Behavior *)(behaviorList.GetElementAtIndex(1));
        }
        else
        {
            nextBehaviorIndex = -1;
            nextBehavior = NULL;
        }
    }

    //cerr << "Attaching VIP behavior: ";
    //cerr << "current index = " << currentBehaviorIndex << ", next index = " << nextBehaviorIndex << endl;

    return currentBehaviorIndex;
}

// loop thru list of all behaviors checking to see which ones need to be updated (re-rendered haptically)
void Vip::updateBehaviors()
{
    void *fn;
    VIP_Behavior* b;
    int cur_beh_index = 1;

    behaviorList.ResetIterator();   // behaviorList may not be threadsafe...need to check this out

    while( (fn = behaviorList.GetNext()) != NULL )
    {
        b = (VIP_Behavior*)fn;
        if( b->requiresUpdate() )
            b->updateBehavior(cur_beh_index);

        cur_beh_index++;
    }
}

void Vip::editPathPoint(int i, const Point3d val, bool isEndPt, int behavior_index)
{
    long mask = ChangeTranslate;

    if( behavior_index > 0 && behavior_index <= behaviorList.Size() )
    {
        vip_path->modifyPoint(i-1, val, mask, behavior_index-1); // DataLines uses zero-based line records

        // is there a next behavior and does it need to be updated
        if( isEndPt && behavior_index < behaviorList.Size() )
        {
            int next_behavior_index = behavior_index+1;
            VIP_Behavior* next_behavior = (VIP_Behavior *)(behaviorList.GetElementAtIndex(next_behavior_index));
            next_behavior->editStartPoint(val, next_behavior_index);
        }
    }
    else // assume current behavior
    {
        if( currentBehaviorIndex != -1 )
        {
            vip_path->modifyPoint(i-1, val, mask, currentBehaviorIndex-1); // DataLines uses zero-based line records
        }

        // adjust start point of next behavior, if applicable
        if( isEndPt && nextBehaviorIndex != -1 )
        {
            nextBehavior->editStartPoint(val, nextBehaviorIndex);
            //nextBehavior->setStartPoint(val); // only works for Transit behavior (ie, 2 points in behavior)
            //vip_path->modifyPoint(0, val, mask, nextBehaviorIndex-1); // DataLines uses zero-based line records
        }
    }
}

void Vip::getPathPointPosition(int i, Point3d& pos, int behavior_index)
{
    if( behavior_index > 0 && behavior_index <= behaviorList.Size() )
    {
        vip_path->getPoint(i-1, pos, behavior_index-1); // DataLines uses zero-based line records
    }
    else // assume current behavior
    {
        if( currentBehaviorIndex != -1 )
        {
            vip_path->getPoint(i-1, pos, currentBehaviorIndex-1); // DataLines uses zero-based line records
        }
    }
}

void Vip::addPathPoint(int p, int behavior_index)
{
//  long mask = ChangeTranslate;

    if( behavior_index > 0 && behavior_index <= behaviorList.Size() )
    {
        vip_path->insertPoint(p-1, behavior_index-1);
    }
    else  // assume current behavior
    {
        if( currentBehaviorIndex != -1 )
        {
            vip_path->insertPoint(p-1, currentBehaviorIndex-1);
        }
    }
}

void Vip::deletePathPoint(int p, int behavior_index)
{
//  long mask = ChangeTranslate;

    if( behavior_index > 0 && behavior_index <= behaviorList.Size() )
    {
        vip_path->deletePoint(p-1, behavior_index-1);
    }   
    else  // assume current behavior
    {
        if( currentBehaviorIndex != -1 )
        {
            vip_path->deletePoint(p-1, currentBehaviorIndex-1);
        }
    }
}

void Vip::addBehavior(VIP_Behavior* current_beh, VIP_Behavior* new_beh)
{
    int i;

    // reset visual aspect of previous behavior line
    if( vip_path )
        vip_path->modifyLineAttrs(2.0, palette[WHITE], currentBehaviorIndex-1);

    // insert new behavior after current behavior
    i = behaviorList.InsertAfterFirstElement(current_beh, new_beh);
    if( i != -1 )
        cerr << " New behavior added at index " << i << endl;
    else
        cerr << " ERROR: Failure to add new behavior!" << endl;

    // update corresponding line record in DataLines object
    if( vip_path == NULL )
    {
        vip_path = new DataLines();
        vip_path->Init();
        vip_path->SetLabel("Path");
        vip_path->insertLine(1, i); // 2 point line to begin...may have to fold this into Init() for lines>2 points
        mainWindow->addObject(vip_path);
    }
    else
        vip_path->insertLine(1, i); // 2 point line to begin...may have to fold this into Init() for lines>2 points

    // Initialize point positions -  haptics loop will also call attachCurrentBehavior() after this (in attach())
    attachCurrentBehavior(new_beh);
    new_beh->initBehavior();
}

// works on the last attached behavior
void Vip::deleteCurrentBehavior()
{
    // delete behavior from behaviorList (if not NULL)
    if( currentBehavior != NULL )
    {
        behaviorList.RemoveFirst(currentBehavior);

        // remove line from vip_path OR delete vip_path if behaviorList.Size() is zero
        if( behaviorList.Size() > 0 )
        {
            vip_path->deleteLine(0, currentBehaviorIndex-1);

            if( nextBehavior != NULL ) // bring next behavior's start point in sync with previous behavior endpoint
            {
                nextBehavior->editStartPoint(*(currentBehavior->getStartPoint()), nextBehaviorIndex-1);
                updateBehaviors();
            }
        }
        else
        {
            vip_path->Unload();
            vip_path = NULL;
        }

        currentBehaviorIndex = -1;
        currentBehavior = NULL;
    }
}

void Vip::deleteMission()
{
    void *fn;
    VIP_Behavior* b;
    Object* ob;

    behaviorList.ResetIterator();
    while( (fn = behaviorList.GetNext()) != NULL )
    {
        b = (VIP_Behavior*)fn;          // static cast back to type as passed into list (before converted to void*)
        DYNAMIC_CAST(ob, Object*, b)    // dynamic cast to Object parent type to allow Unload()
        ob->Unload();
    }
    behaviorList.Clear();

    detachCurrentBehavior();

    if( vip_path )
    {
        vip_path->Unload();
        vip_path = NULL;
    }
}*/

/*---------------------------------------------------------------------------*/
// documented in Object
int Vip::readFrom(FILE *infile)
{
    //parse header
    if (Object::readFrom(infile))
        return 1;

    return 0; //returns 0 if ok.
}

/*--------------------------------------------------------------------------*/
int Vip::processField(int option, char **fields)
{
    int st;
    int i = 1;

    switch(option)
    {
        case VEHICLE:
        case INSTRUMENT:
        case PLATFORM:
            type = option;
            st = Lookup(fields[i++]);
            if( st != -1 )
                subtype = st;
            break;
        case NAME:
            SetLabel(fields[i++]);
            break;
        case SPEED:
//          minSpeed = ConvertFloatAt(i++);
//          maxSpeed = ConvertFloatAt(i++);
            break;
        default:
            return Object::processField(option, fields);
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
int Vip::writeTo(FILE *file)
{
    fprintf(file, "%s %s\n", Keyword(type), Keyword(subtype));
    fprintf(file, "NAME \"%s\"\n", GetLabel().c_str());

    Object::writeTo(file);

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**    Display info about this object in the info window on the GUI.
 */
void Vip::ShowInfo(GLuint *names, int numNames)
{
    char infoLine[512];
    Scene3D *scene;

    scene = mainWindow->getScene3D();

    Object::ShowInfo(names,numNames);

    sprintf(infoLine, "Type:\t%s\n", getRegistrationEntry()->className.c_str());
    scene->appendInfo(infoLine);
    sprintf(infoLine, "Name:\t%s\n", GetLabel().c_str());
    scene->appendInfo(infoLine);
    sprintf(infoLine, "Class:     %s\n", Keyword(type));
    scene->appendInfo(infoLine);
    sprintf(infoLine, "Sub-class: %s\n", Keyword(subtype));
    scene->appendInfo(infoLine);

    gz4d::Point<double> pos = pose->getPosition();
    Point3d vip_pos;
    vip_pos[X] = pos[X];
    vip_pos[Y] = pos[Y];
    vip_pos[Z] = -1.0*(pos[Z]);

    sprintf(infoLine, "Pos: \\[N\\] %11.1f m\n", vip_pos[Y]);
    scene->appendInfo(infoLine);
    sprintf(infoLine, "     \\[E\\] %11.1f m\n", vip_pos[X]);
    scene->appendInfo(infoLine);
    sprintf(infoLine, "     \\[Depth\\] %7.1f m\n", vip_pos[Z]);
    scene->appendInfo(infoLine);

    sprintf(infoLine, "File: %s\n", (GetFilename().empty())? "<none>" : GetFilename().c_str());
    scene->appendInfo(infoLine);
    scene->appendInfo("--------------------\n");
}

// print path waypoints for the vehicle
/*void Vip::writePath(ostream& stream)
{
    stream << endl;
    stream << GetLabel() << " ";
    stream << "Waypoints file" << endl;

    // get lat and long of SW corner of bathymetry - waypoints are meter offsets from this origin
    double lon, lat;
    Point3d geo_ref;

    COPY_POINT(geo_ref, *(mainWindow->getScene3D()->GetGeoRef())); // assumes gutm is object 0

    UTMzones[9]->makeCurrent();
    UTMzones[9]->invert(geo_ref[X], geo_ref[Y], lon, lat);  // HACK, assumes we are UTM and in zone 9 (nonan_r.gutm)
    stream << "Lat: " << lat << endl;
    stream << "Lon: " << lon << endl;

    stream << "[North] [East] [Depth]" << endl;

    // write out VIP start location
    const GLfloat *pos = pose->getPosition();
    stream << pos[Y] << " " << pos[X] << " " << -(geo_ref[Z] + pos[Z]) << endl;

    void *fn;
    VIP_Behavior* b;
    int cur_beh_index = 1;
    Point3d point_pos;

    behaviorList.ResetIterator();   // behaviorList may not be threadsafe...need to check this out
    while( (fn = behaviorList.GetNext()) != NULL )
    {
        b = (VIP_Behavior*)fn;

        // write out behavior label and path speed
        if( b->getType() == VIP_Behavior::TRANSIT )
            stream << "TRANSIT " << b->getPathSpeed() << endl;
        else if( b->getType() == VIP_Behavior::LAWNMOWER_SURVEY )
            stream << "LAWNMOWER_SURVEY " << b->getPathSpeed() << endl;

        // loop over DataLines line at this behavior index, write out waypoints
        for( int i=1; i<vip_path->getNumPoints(cur_beh_index-1); i++ )
        {
            vip_path->getPoint(i, point_pos, cur_beh_index-1); // DataLines uses zero-based line records
            stream << point_pos[Y] << " " << point_pos[X] << " " << -(geo_ref[Z] + point_pos[Z]) << endl;
        }

        cur_beh_index++;
    }

    stream << endl;
}*/

/*---------------------------------------------------------------------------*/
/** This function renders the object in the passed window.  The degradation
 *  parameter informs the object to render itself more or less quickly.  A
 *  degradation of 0 indicates no degradation (high-res, for instance), while
 *  a degradation of 1 indicates maximum degradation.  The Window3D parameter
 *  can be used to determine information such as pixel-size-in-meters, etc.
 */
void Vip::DrawLowRes(float degradation, Window3D *win)
{
//  float winScale;
    float scale;

    doneFlag = 0;
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT);
    if(!IsEmpty())
    {
        ObjectCollection::DrawLowRes(degradation,win);
        glPopMatrix();//get back to scene coordinates
            tail.DrawIn(win);
        glPushMatrix();
        return;
    }

    scale = 1.0;
    
    glShadeModel(GL_SMOOTH);
    if (type == VEHICLE)
    {
        if (subtype == ABE)
        {
            glColor3f(1.0,1.0,1.0);
            glPushMatrix();
                glTranslatef(0.0, 0.0, -1.0*scale);
                glScalef(0.55*scale, 3.0 * scale, 0.65*scale);
                glRotatef(90.0,1.0,0.0,0.0);
                cylinder.draw(degradation);
            glPopMatrix();
            glColor3f(0.8f,0.0,0.0);
            glPushMatrix();
                glTranslatef(-1.25*scale, 0.0, 2.0*scale);
                glScalef(scale, 5.0 * scale, scale);
                glutSolidSphere(0.75,10,10);
            glPopMatrix();
            glPushMatrix();
                glTranslatef(1.25*scale, 0.0, 2.0*scale);
                glScalef(scale, 5.0 * scale, scale);
                glutSolidSphere(0.75,10,10);
            glPopMatrix();
        }
        else if (subtype == TORPEDO)
        {
            //glColor3f(0.5f, 1.0f, 0.0f);
            glColor3f(0.7f, 1.0f, 1.0f);
            //glColor3f(0.3f, 0.3f, 0.8f);
            glPushMatrix();
                //scale = 10 / sqrt(winScale);
                //scale = 1.0;
                if(width)
                {
                    glScalef(width, length, height);
                }
                else
                {
                    glScalef(scale, 4 * scale, scale);
                }
                //glScalef(1.5f,9.0,1.5f);
                //glutSolidSphere(1.0,10,10);
                glPointSize(3);
                glBegin(GL_POINTS);
                glVertex3f(0.0,0.0,0.0);
                glEnd();
                glutSolidSphere(1.0,10,10);
                //&&&fins
                glBegin(GL_TRIANGLES);
                    glNormal3f(0.0, 0.1f, 1.0);
                    glVertex3f(-1.0, -1.0, 0.0);
                    glVertex3f(0.0, 0.0, 0.0);
                    glVertex3f(1.0, -1.0, 0.0);
                    glNormal3f(0.1f, 1.0, 0.0);
                    glVertex3f(0.0, -1.0, -1.0);
                    glVertex3f(0.0, 0.0, 0.0);
                    glVertex3f(0.0, -1.0, 1.0);
                glEnd();
            glPopMatrix();
            glEnable(GL_LIGHTING);
            glDisable(GL_LINE_SMOOTH);
            glPopMatrix();//get back to scene coordinates
            tail.DrawIn(win);
            glPushMatrix();
        }
        else if(subtype == OPEN_SPACE_FRAME)
        {
            //make visible at all scales
            glPointSize(9);
            glColor3f(0.8f,0.8f,0.8f);
            glBegin(GL_POINTS);
                glVertex3f(0.0,0.0,0.0);
            glEnd();

            // make this look like a JasonII for now...

            // bottom gray part
            glColor3f(0.5,0.5,0.5);
            //left side
            glCullFace(GL_BACK);
            glEnable(GL_CULL_FACE);
            glBegin(GL_POLYGON);
                glNormal3f(-1.0,0.0,0.0);
                glVertex3d(-1.125,1.7,0.49);
                glVertex3d(-1.125,1.7,-1.22);
                glVertex3d(-1.125,-1.5,-1.22);
                glVertex3d(-1.125,-1.7,-1.02);
                glVertex3d(-1.125,-1.7,0.49);
            glEnd();
            // right side
            glBegin(GL_POLYGON);
                glNormal3f(1.0,0.0,0.0);
                glVertex3d(1.125,-1.7,0.49);
                glVertex3d(1.125,-1.7,-1.02);
                glVertex3d(1.125,-1.5,-1.22);
                glVertex3d(1.125,1.7,-1.22);
                glVertex3d(1.125,1.7,0.49);
            glEnd();
            // bottom
            glBegin(GL_POLYGON);
                glNormal3f(0.0,0.0,-1.0);
                glVertex3d(-1.125,1.7,-1.22);
                glVertex3d(1.125,1.7,-1.22);
                glVertex3d(1.125,-1.5,-1.22);
                glVertex3d(-1.125,-1.5,-1.22);
            glEnd();
            //front
            glBegin(GL_POLYGON);
                glNormal3f(0.0,1.0,0.0);
                glVertex3d(-1.125,1.7,0.49);
                glVertex3d(1.125,1.7,0.49);
                glVertex3d(1.125,1.7,-1.22);
                glVertex3d(-1.125,1.7,-1.22);
            glEnd();
            //back
            glBegin(GL_POLYGON);
                glNormal3f(0.0,-1.0,0.0);
                glVertex3d(-1.125,-1.7,0.49);
                glVertex3d(-1.125,-1.7,-1.02);
                glVertex3d(1.125,-1.7,-1.02);
                glVertex3d(1.125,-1.7,0.49);
            glEnd();
            // angled back
            glBegin(GL_POLYGON);
                glNormal3f(0.0,-1.0,-1.0);
                glVertex3d(-1.125,-1.7,-1.02);
                glVertex3d(-1.125,-1.5,-1.22);
                glVertex3d(1.125,-1.5,-1.22);
                glVertex3d(1.125,-1.7,-1.02);
            glEnd();

            //Blue mid section
            glColor3f(0.0,0.0,1.0);
            //left side
            glBegin(GL_POLYGON);
                glNormal3f(-1.0,0.0,0.0);
                glVertex3d(-1.125,1.7,0.49);
                glVertex3d(-1.125,-1.7,0.49);
                glVertex3d(-1.125,-1.7,0.84);
                glVertex3d(-1.125,1.7,0.84);
            glEnd();
            //right side
            glBegin(GL_POLYGON);
                glNormal3f(1.0,0.0,0.0);
                glVertex3d(1.125,1.7,0.49);
                glVertex3d(1.125,1.7,0.84);
                glVertex3d(1.125,-1.7,0.84);
                glVertex3d(1.125,-1.7,0.49);
            glEnd();
            // front
            glBegin(GL_POLYGON);
                glNormal3f(0.0,1.0,0.0);
                glVertex3d(-1.125,1.7,0.49);
                glVertex3d(-1.125,1.7,0.84);
                glVertex3d(1.125,1.7,0.84);
                glVertex3d(1.125,1.7,0.49);
            glEnd();
            // back
            glBegin(GL_POLYGON);
                glNormal3f(0.0,-1.0,0.0);
                glVertex3d(-1.125,-1.7,0.49);
                glVertex3d(1.125,-1.7,0.49);
                glVertex3d(1.125,-1.7,0.84);
                glVertex3d(-1.125,-1.7,0.84);
            glEnd();

            // Yellow top
            glColor3f(1.0,1.0,0.0);
            glBegin(GL_POLYGON);
                glNormal3f(-1.0,0.0,0.0);
                glVertex3d(-1.125,1.7,0.84);
                glVertex3d(-1.125,-1.7,0.84);
                glVertex3d(-1.125,-1.2,1.19);
                glVertex3d(-1.125,1.7,1.19);
            glEnd();
            //right side
            glBegin(GL_POLYGON);
                glNormal3f(1.0,0.0,0.0);
                glVertex3d(1.125,1.7,0.84);
                glVertex3d(1.125,1.7,1.19);
                glVertex3d(1.125,-1.2,1.19);
                glVertex3d(1.125,-1.7,0.84);
            glEnd();
            //front
            glBegin(GL_POLYGON);
                glNormal3f(0.0,1.0,0.0);
                glVertex3d(-1.125,1.7,0.84);
                glVertex3d(-1.125,1.7,1.19);
                glVertex3d(1.125,1.7,1.19);
                glVertex3d(1.125,1.7,0.84);
            glEnd();
            //top
            glBegin(GL_POLYGON);
                glNormal3f(0.0,0.0,1.0);
                glVertex3d(-1.125,1.7,1.19);
                glVertex3d(-1.125,-1.2,1.19);
                glVertex3d(1.125,-1.2,1.19);
                glVertex3d(1.125,1.7,1.19);
            glEnd();
            // angled back
            glBegin(GL_POLYGON);
                glNormal3f(0.0,-1.0,1.0);
                glVertex3d(-1.125,-1.7,0.84);
                glVertex3d(1.125,-1.7,0.84);
                glVertex3d(1.125,-1.2,1.19);
                glVertex3d(-1.125,-1.2,1.19);
            glEnd();
            glDisable(GL_CULL_FACE);
            glPopMatrix();//get back to scene coordinates
                tail.DrawIn(win);
            glPushMatrix();
        }
        else if(subtype == CUSTOM_SPACE_FRAME)
        {
            //make visible at all scales
            glPointSize(9);
            glColor3f(0.8f,0.8f,0.8f);
            glBegin(GL_POINTS);
                glVertex3f(0.0,0.0,0.0);
            glEnd();

            glColor3f(0.8f,0.8f,0.8f);
            //left side
            glCullFace(GL_BACK);
            glEnable(GL_CULL_FACE);
            glBegin(GL_POLYGON);
                glNormal3f(-1.0,0.0,0.0);
                glVertex3d(-width/2.0,-length/2.0,height/2.0);
                glVertex3d(-width/2.0,length/2.0,height/2.0);
                glVertex3d(-width/2.0,length/2.0,-height/2.0);
                glVertex3d(-width/2.0,-length/2.0,-height/2.0);
            glEnd();
            // right side
            glBegin(GL_POLYGON);
                glNormal3f(1.0,0.0,0.0);
                glVertex3d(width/2.0,length/2.0,height/2.0);
                glVertex3d(width/2.0,-length/2.0,height/2.0);
                glVertex3d(width/2.0,-length/2.0,-height/2.0);
                glVertex3d(width/2.0,length/2.0,-height/2.0);
            glEnd();
            // top
            glBegin(GL_POLYGON);
                glNormal3f(0.0,0.0,1.0);
                glVertex3d(width/2.0,length/2.0,height/2.0);
                glVertex3d(-width/2.0,length/2.0,height/2.0);
                glVertex3d(-width/2.0,-length/2.0,height/2.0);
                glVertex3d(width/2.0,-length/2.0,height/2.0);
            glEnd();
            // bottom
            glBegin(GL_POLYGON);
                glNormal3f(0.0,0.0,-1.0);
                glVertex3d(width/2.0,-length/2.0,-height/2.0);
                glVertex3d(-width/2.0,-length/2.0,-height/2.0);
                glVertex3d(-width/2.0,length/2.0,-height/2.0);
                glVertex3d(width/2.0,length/2.0,-height/2.0);
            glEnd();
            // front
            glBegin(GL_POLYGON);
                glNormal3f(0.0,1.0,0.0);
                glVertex3d(width/2.0,length/2.0,height/2.0);
                glVertex3d(width/2.0,length/2.0,-height/2.0);
                glVertex3d(-width/2.0,length/2.0,-height/2.0);
                glVertex3d(-width/2.0,length/2.0,height/2.0);
            glEnd();
            // back
            glBegin(GL_POLYGON);
                glNormal3f(0.0,-1.0,0.0);
                glVertex3d(-width/2.0,-length/2.0,height/2.0);
                glVertex3d(-width/2.0,-length/2.0,-height/2.0);
                glVertex3d(width/2.0,-length/2.0,-height/2.0);
                glVertex3d(width/2.0,-length/2.0,height/2.0);
            glEnd();
            //arrow
            glColor3f(1.0,0.5,0.5);
            glBegin(GL_LINES);
                glVertex3d(0.0,length/2.0,height*0.51);
                glVertex3d(0.0,-length/2.0,height*0.51);
                glVertex3d(0.0,length/2.0,height*0.51);
                glVertex3d(-width/2.0,length/4.0,height*0.51);
                glVertex3d(0.0,length/2.0,height*0.51);
                glVertex3d(width/2.0,length/4.0,height*0.51);
            glEnd();
            glDisable(GL_CULL_FACE);
            glPopMatrix();//get back to scene coordinates
                tail.DrawIn(win);
            glPushMatrix();
        }
    }
    else if (type == PLATFORM)
    {
        if(subtype == OPEN_SPACE_FRAME)
        {
            if (renderingForPicking)
            {
                glutSolidCube(1);
            }
            else
            {
                glColor3f(1.0f, 1.0f, 1.0f);
                glDisable(GL_LIGHTING);
                glLineWidth(2);
                glutWireCube(1);
                // make it visible from far away;
                glPointSize(5);
                glBegin(GL_POINTS);
                    glVertex3f(0.0,0.5,0.5);
                glEnd();
                // arrow
                glBegin(GL_LINES);
                    glVertex3f(0.0,-0.5,0.5);
                    glVertex3f(0.0,0.5,0.5);
                    glVertex3f(0.0,0.5,0.5);
                    glVertex3f(-0.5,0.2f,0.5);
                    glVertex3f(0.0,0.5,0.5);
                    glVertex3f(0.5,0.2f,0.5);
                glEnd();
                glEnable(GL_LIGHTING);
                glPopMatrix();//get back to scene coordinates
                    tail.DrawIn(win);
                glPushMatrix();
            }
        }
        else
        {
            glColor3f(0.7f, 1.0f, 1.0f);
//      glColor3f(0.3f, 0.3f, 0.8f);
            glPushMatrix();
                glTranslatef(0.0, 0.0, -10.0*scale);
                cylinder.draw(degradation);
                glPushMatrix();
                    glTranslatef(0.0, 0.0, 10.0*scale);
                    glScalef(0.1f*scale, 0.1f*scale, 7.0*scale);
                    cylinder.draw(0);//degradation);
                glPopMatrix();
                glPushMatrix();
                    glTranslatef(0.0, 0.0, 10.0*scale);
                    glRotatef(90, 0.0, 1.0, 0.0);
                    glScalef(0.5f*scale, 0.5f*scale, 0.5f*scale);
                    cone.draw(0);//degradation);
                glPopMatrix();
                glTranslatef(0.0, 0.0, 20.0*scale);
                glutSolidSphere(1.0, 10, 10);
            glPopMatrix();
        }
    }

}

/*---------------------------------------------------------------------------*/
int Vip::DrawHighRes(int eye, Window3D *win)
{
    DrawLowRes(0, win);

    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
        doneFlag = 1;

    return 1;
}

/*--------------------------Picking Functions--------------------------------*/
/*---------------------------------------------------------------------------*/
int Vip::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
    return Object::pickAction(pickType, inputInfo, names, hits);
}

/*---------------------------------------------------------------------------*/
int Vip::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    const char *message = argv[0];

    if (argc >= 1)
    {
        if (!strcmp(message, "truncateTail"))
        {
            if (argc < 2)
            {
                Tcl_AppendResult(interp, "usage: truncateTail <index>", NULL);
                return TCL_ERROR;
            }
            tail.truncateTailAt(atoi(argv[1]));
            return TCL_OK;
        }
    }
    return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Vip::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "truncateTail ", NULL);
    return Object::list(interp);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Vip::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{

    if ((whatChanged & ChangeTranslateMask) && pose)
    {
        tail.newPosition(pose->getPosition());
    }
    Object::PoseChanged(whichPose, whatChanged);
}


Object * Vip::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    std::string text;

    Object::LoadXML(xml,path,ncfile);

    dslObject = xml.get("<xmlattr>.DSL",dslObject);
    width = xml.get("<xmlattr>.width",width);
    height = xml.get("<xmlattr>.height",height);
    length = xml.get("<xmlattr>.length",length);
    datafilename = xml.get("<xmlattr>.datafile",datafilename);
    boost::optional<std::string> attribute = xml.get_optional<std::string>("<xmlattr>.datatype");
    if(attribute && *attribute == "Remus")
        datatype = Remus;

    if(!datafilename.empty())
    {
        datafile = std::fopen((fs::path(path)/datafilename).string().c_str(),"r");
        if(!datafile)
        {
            std::cerr << "Vip: error opening datafile: " << datafilename << std::endl;
        }
        else if(datatype == Remus)
        {
            mainWindow->getScene3D()->timeControl->setRTsource(this);
            mainWindow->getScene3D()->timeControl->useRT();
            projectionRequired = GL_TRUE;
        }
    }

    ObjectCollection::LoadXML(xml,path,ncfile);
    return this;
}

void Vip::Idle(double time)
{
    char line[2048];
    int hours,minutes,days;
    float seconds;
    double tm;
    double x,y,z;

    if(datafile && datatype==Remus)
    {
        if(feof(datafile))
        {
            fclose(datafile);
            datafile = NULL;
        }
        else if(fgets(line,2048,datafile))
        {
            if(sscanf(line,"%lf, %*f, %i:%i:%f, %i, %lf, %lf,",&z, &hours, &minutes, &seconds, &days, &y, &x)==7)
            {
                tm = seconds + days*60*60*24 + hours*60*60 + minutes*60;
                z = -z;
                autoProjection(x,y);
                gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                gz4d::Point<double> xy = proj(y,x);
                SetPosition(xy[0],xy[1],z,tm);
                currentTime = tm;
                bbox.expandTime(currentTime);
                if(parent)
                    parent->setBBoxStale();
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
        }
    }
    
    if(!dsl && !dslObject.empty())
    {
        dsl = (DSL *)(mainWindow->getScene3D()->GetObjectPtr(dslObject));
        mainWindow->getScene3D()->timeControl->setRTsource(this);
        mainWindow->getScene3D()->timeControl->useRT();
        calculateCOG=0;
    }

    Object::Idle(time);

}


















