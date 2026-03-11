// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// ProximityHighlighter.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides a way to detect close proximity among a selected
 *	set of reference frames, and lock a view on two close-proximity objects.
 *	Scale is updated such that the positions of the two nearest objects are
 *	present in the center of workspace of any ViewManager attached to this.
 *	This class implements the following responsibilities.
 *		* Managing PosableStateBases by collecting change events
 *		  and keeping the result pose updated.
 *		* Rendering to the screen (renders a wire box, if visible)
 *		* Loading and saving from/to disk
 *
 *	Created by mdp 6/28/2001
 */
/*---------------------------------------------------------------------------*/

#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "engine/PosableStateRelationships/ProximityHighlighter.h"
#include "widgets/Sv_Standards.h"
#include "engine/ViewManager.h"
#include "engine/InputMediator.h"
#include "engine/PosableStateBase.h"
#include <stdio.h>

REGISTER_OBJECT(ProximityHighlighter, "proximity");

#define SQRT3 1.732f
#define DEFAULT_THRESHOLD 60 //30 meters
#define MATRIX_QUANTUM 5
#define MIN_COL 0

extern MainWindow *mainWindow;

const char *ProximityNames[] = {COMPOSITE_OBJECT_NAMES, "THRESHOLD", "TRACKING", ""};

const EventType *ProximityHighlighter::inProximityInputType =
				Interactor::newEventType("inProximity", Interactor::DataAsAddress);
const EventType *ProximityHighlighter::inCloseProximityInputType =
				Interactor::newEventType("inCloseProximity", Interactor::DataAsAddress);
const EventType *ProximityHighlighter::outOfProximityInputType =
				Interactor::newEventType("outOfProximity", Interactor::DataAsAddress);
const EventType *ProximityHighlighter::outOfCloseProximityInputType =
				Interactor::newEventType("outOfCloseProximity", Interactor::DataAsAddress);
const EventType *ProximityHighlighter::proximityHighlightChangeInputType =
				Interactor::newEventType("proximityHighlightChange", Interactor::DataAsAddress);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
ProximityHighlighter::ProximityHighlighter(void *creator)
: CompositeObject(creator)
{
	// set up the reader to look for BEGIN AVERAGING_OVERVIEW 1.0,
	// and with set of keywords
	SetTypeAndVersion("PROXIMITY_HIGHLIGHTER", 1, 0);
	SetKeywordTable(ProximityNames);

	//default to all on.
	relationshipFromOperands = TranslateComplex;
	relationshipToResult = TranslateComplex | ScaleComplex;

	//default to visible.
	visible = GL_TRUE;

	//empty the min/max info.
	matrixSize = 0;
	distanceToNeighbor = NULL;
	cellWithMin = NULL;
	closestIndexRow = closestIndexColumn = -1;
	threshold = DEFAULT_THRESHOLD;
	alarmState = AlarmSafe;
	scaleFactor = 1.0;
	readingFile = GL_FALSE;
	debugFlag = GL_FALSE;

}

/*---------------------------------------------------------------------------*/
ProximityHighlighter::~ProximityHighlighter()
{
	if (distanceToNeighbor)
		delete distanceToNeighbor;
	if (cellWithMin)
		delete cellWithMin;
}

/*---------------------------Interaction Facilitation------------------------*/
/*---------------------------------------------------------------------------*/
void ProximityHighlighter::PoseChanged(PosableStateBase *whichPose,
									long whatChanged)
{
	int i;

	if ((whatChanged & ChangeCurrentMask) == 0) //only care about "current"
		return;

	if (whichPose == pose)
		Object::PoseChanged(whichPose, whatChanged);
	else //update our table.
	{
		for (i = 0; i < numOperands; i++)
		{
			if (poseOfOperands[i] == whichPose)
				break;
		}
		if (i >= numOperands) //wasn't one of ours!
			return;
		updateMinTable(i);
	}
}

/*---------------------------------------------------------------------------*/
/**	This procedure doesn't do any smart updating, it just recalculates the
 *	whole table for finding minimum distances between managed
 *	PosableStateBases.  The matrix looks like this:
 *
 *	                distanceToNeighbor                  cellWithMin
 *	==================================================  ===========
 *	   |    0   |    1   |    2   |    3   |    4   |
 *	---+--------+--------+--------+--------+--------+-  -+--------
 *	 0 | min(0) | d(0,1) | d(0,2) | d(0,3) | d(0,4) |   0| ColOf(min(0))
 *	---+--------+--------+--------+--------+--------+-  -+--------
 *	 1 | min(1) |        | d(1,2) | d(1,3) | d(1,4) |   1| ColOf(min(1))
 *	---+--------+--------+--------+--------+--------+-  -+--------
 *	 2 | min(2) |        |        | d(2,3) | d(2,4) |   2| ColOf(min(2))
 *	---+--------+--------+--------+--------+--------+-  -+--------
 *	 3 | min(3) |        |        |        | d(3,4) |   3| ColOf(min(3))
 *	---+--------+--------+--------+--------+--------+-  -+--------
 */
void ProximityHighlighter::updateMinTable()
{
	int i, j;
	float val, rowMin, totalMin;
	gz4d::Point<double> source, dest;
	double x, y, z;
	int cellAvecMin, rowWithMin; //avec is with in French... ran out of names.

	//reallocate space if now too small
	if (numOperands >= matrixSize)
	{
		if (distanceToNeighbor) //destroy and rebuild from scratch.
		{
			delete distanceToNeighbor;
			delete cellWithMin;
		}
		while (numOperands >= matrixSize)
			matrixSize += MATRIX_QUANTUM;
		distanceToNeighbor = new float[(matrixSize - 1) * matrixSize];
/*		//debugging code
		for (i = 0; i < (matrixSize - 1) * matrixSize; i++)
			distanceToNeighbor[i] = -1;
*/
		cellWithMin = new int[matrixSize];
	}

	//Rebulid distance values and min information from scratch
	rowWithMin = -1;
	totalMin = DEFAULT_THRESHOLD * threshold; //just pick something big enough
	for (i = 0; i < numOperands; i++)
	{
		rowMin = DEFAULT_THRESHOLD * threshold; //just pick something big enough
		cellAvecMin = i + 1;
		source = poseOfOperands[i]->getPosition();
		for (j = i + 1; j < numOperands; j++)
		{
			dest = poseOfOperands[j]->getPosition();
			x = dest[X] - source[X];
			y = dest[Y] - source[Y];
			z = dest[Z] - source[Z];
			val = (float)sqrt(x*x + y*y + z*z);
			distanceToNeighbor[i * matrixSize + j] = val;
			if (val < rowMin)
			{
				cellAvecMin = j;
				rowMin = val;
			}
		}
		distanceToNeighbor[i * matrixSize + MIN_COL] = rowMin;
		cellWithMin[i] = cellAvecMin;
		if (rowMin < totalMin)
		{
			rowWithMin = i;
			totalMin = rowMin;
		}
	}
	for (; i < matrixSize; i++)
		cellWithMin[i] = -1;

	//what is the overall minimum?
	minDistance = totalMin;
	closestIndexRow = rowWithMin;
	closestIndexColumn = cellWithMin[rowWithMin];

	updatePose();
}

/*---------------------------------------------------------------------------*/
/**	This procedure does some smart updating, trying just to recalculate the
 *	distances and minimums needed.  See the comment for updateMinTable()
 *	(no arguments) for what the data structure looks like.
 */
void ProximityHighlighter::updateMinTable(int index)
{
	int i, j, minColIndex, rowOffset, winner, neighbor;
	float val, min, totalMin = minDistance;
        gz4d::Point<double> source, dest;
	double x, y, z;
	int totalMinHolder = closestIndexRow;
	int oldRow = closestIndexRow, oldCol = closestIndexColumn;
	bool updated;
	
	winner = 0;
    min = 0.0;
    
	source = poseOfOperands[index]->getPosition();
	//update Column
	for (i = 0; i < index; i++)
	{
		rowOffset = i * matrixSize;

		dest = poseOfOperands[i]->getPosition();
		x = dest[X] - source[X];
		y = dest[Y] - source[Y];
		z = dest[Z] - source[Z];
		val = (float)sqrt(x*x + y*y + z*z);
		distanceToNeighbor[rowOffset + index] = val;
		//update min info.
		minColIndex = rowOffset + MIN_COL;
		updated = false;
		if (index == cellWithMin[i] || cellWithMin[i] < 0) //used to be min, still?
		{
			if (val <= distanceToNeighbor[minColIndex])
				distanceToNeighbor[minColIndex] = val;
			else // check everyone in row!
			{
				j = i + 1;
				if (j < numOperands)
				{
					val = distanceToNeighbor[rowOffset + j];
					winner = j;
					for (j++; j < numOperands; j++)
					{
						if (distanceToNeighbor[rowOffset + j] < val)
						{
							val = distanceToNeighbor[rowOffset + j];
							winner = j;
						}
					}
					distanceToNeighbor[minColIndex] = val;
					cellWithMin[i] = winner;
				}
			}
			updated = true;
		}
		min = distanceToNeighbor[rowOffset + cellWithMin[i]];
		//---Following lines added by mdp 4/14/2005---
		if (!updated && (val < min))
		{
			cellWithMin[i] = index;
			min = val;
		}
		//---End of lines added 4/14/2005
		if (i == 0 || min < totalMin)
		{
			totalMin = min;
			totalMinHolder = i;
		}
	}

	//update Row
	neighbor = index + 1;
	if (neighbor < numOperands)
	{
		rowOffset = index * matrixSize;
		minColIndex = rowOffset + MIN_COL;
		for (j = neighbor; j < numOperands; j++)
		{
			dest = poseOfOperands[j]->getPosition();
			x = dest[X] - source[X];
			y = dest[Y] - source[Y];
			z = dest[Z] - source[Z];
			val = (float)sqrt(x*x + y*y + z*z);
			distanceToNeighbor[rowOffset + j] = val;
			if (j == neighbor || val < min)
			{
				min = val;
				winner = j;
			}
		}
		distanceToNeighbor[minColIndex] = min;
		cellWithMin[index] = winner;
		if (min < totalMin)
		{
			totalMin = min;
			totalMinHolder = index;
		}
		else if (min > totalMin && winner == closestIndexColumn && index == closestIndexRow)
		{
			totalMin = min;
			totalMinHolder = index;
		}
	}

	//update minimum across rows.
	if (closestIndexRow <= index && totalMin > minDistance) //have to check rest
	{
		for (i = neighbor; i < (numOperands - 1); i++)
		{
			minColIndex = i * matrixSize + MIN_COL;
			if (distanceToNeighbor[minColIndex] < totalMin)
			{
				totalMin = distanceToNeighbor[minColIndex];
				totalMinHolder = i;
			}
		}
	}
	if (closestIndexRow <= index || totalMin < minDistance)
	{
		minDistance = totalMin;
		closestIndexRow = totalMinHolder;
		closestIndexColumn = cellWithMin[totalMinHolder];
	}

	updatePose();
	if ((oldRow != closestIndexRow || oldCol != closestIndexColumn) && (closestIndexColumn >= 0))
		highLevelEventMediator->Trigger(proximityHighlightChangeInputType->value, poseOfOperands[closestIndexRow], poseOfOperands[closestIndexColumn]);

/*	//debugging code
	for (i = 0; i < numOperands; i++)
	{
		cout << endl;
		for (j = 0; j < numOperands; j++)
		{
			cout << distanceToNeighbor[i * matrixSize + j] << "\t";
		}
		cout << ":" << cellWithMin[i];
	}
	cout << endl << minDistance << " " << closestIndexRow << " " << closestIndexColumn << endl;
	cout << endl;
*/
}

/*---------------------------------------------------------------------------*/
void ProximityHighlighter::updatePose()
{
    gz4d::Point<double> source, dest, local;
	double sum1, sum2, x, y, z;

	if (closestIndexRow < 0 || closestIndexColumn < 0 ||
		closestIndexRow >= numOperands || closestIndexColumn >= numOperands)
		return;

	pose->AccumulateNotifications();
	source = poseOfOperands[closestIndexRow]->getPosition();
	dest = poseOfOperands[closestIndexColumn]->getPosition();
	if (relationshipToResult & Translate)
	{
		pose->setPosition((source[X] + dest[X]) / 2,
						  (source[Y] + dest[Y]) / 2,
						  (source[Z] + dest[Z]) / 2);
		scaleFactor = minDistance / 2.0f; //we split the difference
	}
	else
	{
		local = pose->getPosition();
		x = local[X] - source[X];
		y = local[Y] - source[Y];
		z = local[Z] - source[Z];
		sum1 = x*x + y*y + z*z;
		x = local[X] - dest[X];
		y = local[Y] - dest[Y];
		z = local[Z] - dest[Z];
		sum2 = x*x + y*y + z*z;
		if (sum1 > sum2)
			scaleFactor = (float)sqrt(sum1);
		else
			scaleFactor = (float)sqrt(sum2);
	}
	if (scaleFactor < 1.0)
		scaleFactor = 1.0;
	pose->setScale((float)(scaleFactor / ViewManager::radiusOfWorkspaceCenter));
	pose->ResumeSingularNotification();

	//monitor alarm status.
	if (minDistance > threshold * 1.1)
	{
		if (alarmState != AlarmSafe)
		{
			if (alarmState == AlarmDanger)
			{
				alarmState = AlarmCaution;
				cancelAlarm();
			}
			alarmState = AlarmSafe;
			cancelAlarm();
		}
	}
	else if (minDistance < threshold)
	{
		if (minDistance < threshold / 2.0)
		{
			if (alarmState != AlarmDanger)
			{
				if (alarmState == AlarmSafe) //trigger both on the way in!
				{
					alarmState = AlarmCaution;
					soundAlarm();
				}
				alarmState = AlarmDanger;
				soundAlarm();
			}
		}
		else
		{
			if (alarmState == AlarmSafe)
			{
				alarmState = AlarmCaution;
				soundAlarm();
			}
			else if (alarmState == AlarmDanger && minDistance > (threshold * 0.55))
			{
				alarmState = AlarmCaution;
				cancelAlarm();
			}
		}
	}
}

/*---------------------------------------------------------------------------*/
long ProximityHighlighter::manage(PosableStateBase *pose)
{
	long retval = CompositeObject::manage(pose);

	if (readingFile)
		return retval;

	if (numOperands >= matrixSize)
		updateMinTable();
	else
		updateMinTable(numOperands - 1);

	return retval;
}

/*---------------------------------------------------------------------------*/
void ProximityHighlighter::relinquish(PosableStateBase *pose)
{
	CompositeObject::relinquish(pose);

	updateMinTable();
}

/*---------------------------------------------------------------------------*/
/**	This procedure allows the caller to set the threshold at which
 *	an alarm event will be generated.
 */
void ProximityHighlighter::setThreshold(float newThreshold)
{
	float oldThreshold = threshold;

	threshold = newThreshold;
	if (minDistance < oldThreshold && minDistance > newThreshold)
		cancelAlarm();
	else if (minDistance > oldThreshold && minDistance < newThreshold)
		soundAlarm();
}

/*---------------------------------------------------------------------------*/
/**	This procedure allows the caller to change whether or not position is
 *	tracked (the position of this posable follows the average position of
 *	the two nearest items).
 */
void ProximityHighlighter::trackPosition(GLboolean track)
{
	if (track)
		relationshipToResult |= Translate;
	else
		relationshipToResult &= ~Translate;
}

/*-----------------------------Persistence-----------------------------------*/
/*---------------------------------------------------------------------------*/
int ProximityHighlighter::processField(int option, char **fields)
{
	int i = 1;

	switch(option)
	{
		case THRESHOLD:
			threshold = ConvertFloat(i);
			break;
		case TRACKING:
			if (Lookup(fields[i++]) == OFF)
			{
				trackPosition(GL_FALSE);
				visible = GL_FALSE;
			}
			else
				trackPosition();
			break;
		default:
			return CompositeObject::processField(option, fields);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
/**	This readFrom procedure assumes that the proper number of operands has
 *	been allocated, and that (numOperandsAllocated - numOperands) records
 *	should be read from the file.
 */
int ProximityHighlighter::readFrom(FILE *file)
{
	int retval;
	
	readingFile = GL_TRUE;

	retval = CompositeObject::readFrom(file);

	updateMinTable();
	readingFile = GL_FALSE;

	return retval;
}

/*---------------------------------------------------------------------------*/
int ProximityHighlighter::writeTo(FILE *file)
{
	int tracking;

	if (relationshipToResult & Translate)
		tracking = ON;
	else
		tracking = OFF;

	fprintf(file, "%s %d\n", ProximityNames[N_RECORDS], numOperands);
	fprintf(file, "%s %f\n", ProximityNames[THRESHOLD], threshold);
	fprintf(file, "%s %s\n", ProximityNames[TRACKING], ProximityNames[tracking]);
	return CompositeObject::writeTo(file);
}

/*---------------------------Rendering/Info----------------------------------*/
/*---------------------------------------------------------------------------*/
void ProximityHighlighter::DrawLowRes(float degradation, Window3D *win)
{
	GLfloat dist;

	if (alarmState == AlarmSafe && !debugFlag)
		return;

	dist = 0.5f;
	glColor3f(alarmState - 1.0f, 1.0, 1.0);
	glNormal3f(0.0, 0.0, 1.0);
	glLineWidth((float)alarmState);
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
		//left/right faces
		glVertex3f(-dist, -dist, -dist);
		glVertex3f(-dist, -dist, dist);
		glVertex3f(-dist, dist, dist);
		glVertex3f(-dist, dist, -dist);
		glVertex3f(dist, -dist, -dist);
		glVertex3f(dist, -dist, dist);
		glVertex3f(dist, dist, dist);
		glVertex3f(dist, dist, -dist);
		//front/back faces
		glVertex3f(-dist, -dist, -dist);
		glVertex3f(-dist, -dist, dist);
		glVertex3f(dist, -dist, dist);
		glVertex3f(dist, -dist, -dist);
		glVertex3f(-dist, dist, -dist);
		glVertex3f(-dist, dist, dist);
		glVertex3f(dist, dist, dist);
		glVertex3f(dist, dist, -dist);
		//top/bottom faces
		glVertex3f(-dist, -dist, -dist);
		glVertex3f(-dist, dist, -dist);
		glVertex3f(dist, dist, -dist);
		glVertex3f(dist, -dist, -dist);
		glVertex3f(-dist, -dist, dist);
		glVertex3f(-dist, dist, dist);
		glVertex3f(dist, dist, dist);
		glVertex3f(dist, -dist, dist);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1.0);
	glEnable(GL_LIGHTING);
}

/*---------------------------------------------------------------------------*/
int ProximityHighlighter::DrawHighRes(int eye, Window3D *win)
{
	DrawLowRes(0.0f, win);
	return 1;
}

/*---------------------------------------------------------------------------*/
void ProximityHighlighter::soundAlarm()
{
//	cerr << (char) 7;
//	cerr << (char) 7;
//	if (alarmState == AlarmDanger)
//		cerr << (char) 7;
    if (closestIndexRow < 0 || closestIndexColumn < 0)
        return; //something is majorly wrong... what?
	if (alarmState == AlarmDanger)
		highLevelEventMediator->Trigger(inCloseProximityInputType->value, poseOfOperands[closestIndexRow], poseOfOperands[closestIndexColumn]);
	else
		highLevelEventMediator->Trigger(inProximityInputType->value, poseOfOperands[closestIndexRow], poseOfOperands[closestIndexColumn]);
}

/*---------------------------------------------------------------------------*/
void ProximityHighlighter::cancelAlarm()
{
//	cerr << (char) 7;
	if (alarmState == AlarmCaution)
		highLevelEventMediator->Trigger(outOfCloseProximityInputType->value, poseOfOperands[closestIndexRow], poseOfOperands[closestIndexColumn]);
	else
		highLevelEventMediator->Trigger(outOfProximityInputType->value, poseOfOperands[closestIndexRow], poseOfOperands[closestIndexColumn]);
}

/*---------------------------------------------------------------------------*/
int ProximityHighlighter::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	char retval[128] = "";

	if (argc >= 1)
	{
		if (!strcmp(argv[0], "threshold"))
		{
			if (argc >= 2)
				setThreshold((float)atof(argv[1]));
			sprintf(retval, "%f", threshold);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(argv[0], "debug"))
		{
			if (argc >= 2)
				debugFlag = atoi(argv[1]);
			sprintf(retval, "%i", debugFlag);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
	}
	return CompositeObject::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int ProximityHighlighter::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "threshold debug ", NULL);
	return CompositeObject::list(interp);
}
