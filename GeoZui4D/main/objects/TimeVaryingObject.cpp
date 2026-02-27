/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// TimeVaryingObject.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is a generalized mechanism for creating time-varying objects of any
 *	sort, assuming they know how to load themselves from netCDF.  This object
 *	basically keeps track of a bunch of different versions of the same object,
 *	rendering the appropriate one for the current GeoZui4D time.
 *
 *	Created by mdp 2/11/2004 (by smushing Roland's TVGutm and Tide classes together)
 */
/*---------------------------------------------------------------------------*/

#include "GZincludes.h"
#include "TimeVaryingObject.h"
#include "engine/ToglWindow.h"
#include <string.h>
#include "engine/PosableStateBase.h"
#include <iostream>
#include "engine/InputMediator.h"
#include "engine/TranslatableState.h"
#include <filesystem>


extern MainWindow *mainWindow;

using namespace std;

REGISTER_OBJECT(TimeVaryingObject, "");
REGISTER_ALIAS(TVGutm, TimeVaryingObject, "");
REGISTER_ALIAS(Tide, TimeVaryingObject, "");

/*---------------------------------------------------------------------------*/
TimeVaryingObject::TimeVaryingObject()
{
	currentObject = NULL;
	currentGzTime = 0.0;
	defaultLoadingType = "Gutm";
	treatAsTide = GL_FALSE;
	transparencyLevel = 0;
	timeIndex = NULL;
	numObjects = 0;
}

/*---------------------------------------------------------------------------*/
TimeVaryingObject::~TimeVaryingObject()
{
	delete timeIndex;
}

/*---------------------------------------------------------------------------*/
void TimeVaryingObject::initializeAs(const char* alias)
{
	if (strcmp(alias, "Tide") == 0)
	{
		treatAsTide = GL_TRUE;
		mainWindow->getScene3D()->SetTide(this);
		Object::SetInvisible();
	}
}

/*---------------------------------------------------------------------------*/
Object * TimeVaryingObject::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    double time = 0.0;
    Object * object = NULL;
    int ret;
    BBox b;

    Object::LoadXML(xml,path, ncfile);

    if (ncfile)
    {
        int i;
        int timestepcount = ncfile.GetDim("time").Size();
        ObjectRegistrationEntry *ore;
        string text;
        double *times = NULL;

        times = new double[timestepcount];
        ncfile.GetVar("time").Get(times,timestepcount);

        boost::optional<std::string> attribute;
        //get a default name for ourselves
        if(attribute = xml.get_optional<std::string>("<xmlattr>.ncvar"))
            SetLabel(*attribute);
        else
            return NULL;

        //get a default loading type in case its not a GUTM.
        if(attribute = xml.get_optional<std::string>("<xmlattr>.type"))
        {
            ore = Object::getRegistryEntry(*attribute);
            if (!ore)
                return NULL;
            defaultLoadingType = ore->className;
        }

        //load component objects
        for(i=0;i<timestepcount;i++)
        {
            if (defaultLoadingType == "Tide")
                object = Object::createFromClassname("Gutm");
            else
                object = Object::createFromClassname(defaultLoadingType);
            object->setTimeIndex(i);
            object->LoadXML(xml, path, ncfile);
                        //was object->loadFromNetCDF(ncfile,(char *)xmlstuff,i);
            if (treatAsTide)
            {
                object->ignore(PICK_CLICK1);
                object->ignore(PICK_PRESS3);
                object->setRespondToPress2(255); //shut off automatic changes
            }
            b=object->GetBoundingBox();
            b.start = b.end = times[i];
            object->MergeBBox(b);
            AddObject(object);
            if(!currentObject)
            {
                currentObject = object;
            }

        }

        delete[] times;
        return this;
    }
    else
    {
        for (bpt::ptree::const_iterator node = xml.begin(); node != xml.end(); ++ node)
            if(node->first == "Timestep")
            {
                time = node->second.get("<xmlattr>.time",time);

                boost::optional<std::string> attribute;
                if(attribute = xml.get_optional<std::string>("<xmlattr>.filename"))
                {
                    if (!fs::path(*attribute).extension().string().empty()) //Load the appropriate object based on extension
                    {
                        object = Object::createFromExtension(fs::path(*attribute).extension().string());
                        if (object)
                        {
                            object->ignore(PICK_CLICK1);
                            object->ignore(PICK_PRESS3);
                            object->setRespondToPress2(255); //shut off automatic changes
                        }
                    }

                    if (object != NULL)  // if an object did manage to get created, add it to the list
                    {
                        ret = object->SetFilename((fs::path(path)/(*attribute)).string());
                        if (ret==0)
                        {
                            if (treatAsTide)
                                object->disableProgress();
                            if (object->Load() == 0)
                            {
                                b=object->GetBoundingBox();
                                b.start = b.end = time;
                                object->MergeBBox(b);
                                AddObject(object);
                                if(!currentObject)
                                {
                                    currentObject = object;
                                }
                            }
                            else
                            {
                                delete object;
                                object = NULL;
                            }
                        }
                        else
                        {
                            delete object;
                            object = NULL;
                        }
                    }
                }
            }
    }

    sortAndIndexByTime();
    return this;
}



/*---------------------------------------------------------------------------*/
/** This procedure first sorts the list according to starting times, then
 *	indexes it through timeIndex.  The point is not necessarily to sort the
 *	list as fast as possible, but to make later access very fast.  (So, the
 *	sort takes n^2 time instead of nlgn.) Added by mdp 3/9/2004.
 */
void TimeVaryingObject::sortAndIndexByTime()
{
	double minTime;
	double maxTime;
	double tempTime = 0.0;
	Node *insertCursor, *progressCursor, *previousCursor;
	int i;

	if (!nodeList)
		return;

	previousCursor = nodeList;
	maxTime = minTime = nodeList->object->GetBoundingBox().start;
	numObjects = 1;
	for (progressCursor = nodeList->next; progressCursor; progressCursor = progressCursor->next)
	{
		numObjects++;
		tempTime = progressCursor->object->GetBoundingBox().start;
		if (tempTime < minTime)
		{
			extractNode(progressCursor);
			progressCursor->next = nodeList;
			progressCursor->prev = nodeList->prev;
			nodeList->prev = progressCursor;
			nodeList = progressCursor;
			minTime = tempTime;
			progressCursor = previousCursor;
		}
		else if (tempTime > maxTime)
		{
			maxTime = tempTime;
			previousCursor = progressCursor;
		}
		else //this one belongs in the middle somewhere
		{
			extractNode(progressCursor);
			for (insertCursor = nodeList->next; insertCursor != previousCursor; insertCursor = insertCursor->next)
			{
				if (insertCursor->object->GetBoundingBox().start > tempTime)
					break;
			}
			progressCursor->next = insertCursor;
			progressCursor->prev = insertCursor->prev;
			insertCursor->prev = progressCursor;
			progressCursor->prev->next = progressCursor;
			//previousCursor doesn't change...it's always the max encountered so far, btw
		}
	}
	//at this point, the nodeList should be sorted, numObjects tells us how many
	//there are, and minTime and maxTime tell us the extents.
	indexStep = (maxTime - minTime) / (double) numObjects;
	timeOffset = minTime;
	timeIndex = new Node*[numObjects];
	previousCursor = timeIndex[0] = nodeList;
	progressCursor = nodeList->next;
	if (progressCursor)
		tempTime = progressCursor->object->GetBoundingBox().start;
	for (i = 1; i < numObjects; i++)
	{
		if (((i * indexStep + timeOffset) >= tempTime) && progressCursor)
		{
			previousCursor = progressCursor;
			progressCursor = progressCursor->next;
			tempTime = progressCursor->object->GetBoundingBox().start;
		}
		timeIndex[i] = previousCursor;
	}
}

/*---------------------------------------------------------------------------*/
/**	This procedure removes the passed Node from the list by fixing the
 *	surrounding pointers, but does not alter the passed node.
 *	Added by mdp 3/9/2004.
 */
void TimeVaryingObject::extractNode(Node *n)
{
	if (!n)
		return;

	if (n->prev)
		n->prev->next = n->next;
	if (n->next)
		n->next->prev = n->prev;

	if (nodeList == n)
		nodeList = n->next;
}

/*---------------------------------------------------------------------------*/
/** Here, we should pick the closes object, time wise */
void TimeVaryingObject::Idle(double time)
{
	Object::Idle(time);
	
	Object * oldCurrent = currentObject;
	
	currentObject = GetCurrent(time);
	currentGzTime = time;
	
	if(currentObject != oldCurrent)
	{
		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
}


/*---------------------------------------------------------------------------*/
Object * TimeVaryingObject::GetCurrent(double time)
{
	double closestDistance = 0.0;
	double currentDistance;
	Object * closest;
	Node * cursor;
	Object * currentOb;
	int index, usingSort = GL_FALSE;
	
	closest = currentObject;
	
	if (closest)
	{
		closestDistance = timeDistance(closest->GetBoundingBox(),time);
		if (closestDistance <= 0) //short circuit if alread there! (mdp 2/11/2004)
			return closest;
	}
	
	if(nodeList)
	{
		if (numObjects) //case added by mdp 3/9/2004
		{
			index = (int)(((time - timeOffset) / indexStep) - 0.000001); //.000001 part put in just in case of roundoff error (don't know if it's necessary)
			if (index >= 0 && index < numObjects)
			{
				cursor = timeIndex[index];
				usingSort = GL_TRUE;
			}
			else
				cursor = nodeList;
		}
		else
			cursor = nodeList;
		while (cursor != NULL)
		{
			currentOb = cursor->object;
			currentDistance = timeDistance(currentOb->GetBoundingBox(),time);
			if(currentDistance < closestDistance)
			{
				closestDistance = currentDistance;
				closest = currentOb;
			}
			else if (usingSort) //short circuit it here since we know the list is in order, added by mdp 3/9/2004
				return closest;
			cursor = cursor->next;
		}
	}
	
	return closest;
}

/*---------------------------------------------------------------------------*/
Object * TimeVaryingObject::GetOffsetFromCurrent(double timeOffset)
{
	if (timeOffset == 0.0)
		return currentObject;

	return GetCurrent(currentGzTime + timeOffset);
}

/*---------------------------------------------------------------------------*/
void TimeVaryingObject::DrawLowRes(float degradation, Window3D * win)
{
	DrawObjectLowRes(currentObject, degradation, win);
}

/*---------------------------------------------------------------------------*/
/** How far away, time wise from this bbox, if in bbox, returns 0 */
double TimeVaryingObject::timeDistance(BBox b,double t)
{
	if(b.start<=t && b.end>=t)
		return 0.0;
	if(t<b.start)
	{
		return b.start-t;
	}
	else
	{
		return t-b.end;
	}
}

/*---------------------------------------------------------------------------*/
int TimeVaryingObject::DrawHighRes(int eye, Window3D * win)
{
	return DrawObjectHighRes(currentObject, eye, win);
}


/*--------------------------------------------------------------------------*/
/**	This function returns the height at the passed location for the current
 *	object.  Returns -1.0 if a height is not found at this location.  If a
 *	coordinate array is passed it is filled with the result for an x and y it
 *	knows more exactly.  The "secondary" variable is part of a hack for gettig
 *	at alternateData from scripting.  It is only used when coord is non-NULL.
 */
double TimeVaryingObject::GetHeightAt(double x, double y, Point3d coord, GLboolean secondary)
{
	return currentObject->GetHeightAt(x, y, coord, secondary);
}
