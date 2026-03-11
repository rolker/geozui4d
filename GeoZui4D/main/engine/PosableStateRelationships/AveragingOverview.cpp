// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// AveragingOverview.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides a way to represent an average of multiple reference
 *	frames in their position and rotation attributes.  Scale is updated such
 *	that all positions would be present in the center of workspace of
 *	any ViewManager attached to this (hence the Overview portion).
 *	This class implements the following responsibilities.
 *		* Managing PosableStateBases by collecting change events
 *		  and keeping the result pose updated.
 *		* Rendering to the screen (renders a wire box, if visible)
 *		* Loading and saving from/to disk
 *
 *	Created by mdp 6/22/2001
 */
/*---------------------------------------------------------------------------*/

#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "AveragingOverview.h"
#include "widgets/Sv_Standards.h"
#include "engine/ViewManager.h"
#include "engine/PosableStateBase.h"
#include <stdio.h>

REGISTER_OBJECT(AveragingOverview, "avov");

#define SQRT3 1.732f

extern MainWindow *mainWindow;


const char *AvovNames[] = {COMPOSITE_OBJECT_NAMES, "TRACKING", "AVERAGE",
						   "MEAN", "MIN_MAX", ""};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
AveragingOverview::AveragingOverview(void *creator)
: CompositeObject(creator)
{
	int i;

	// set up the reader to look for BEGIN AVERAGING_OVERVIEW 1.0,
	// and with set of keywords
	SetTypeAndVersion("AVERAGING_OVERVIEW", 1, 0);
	SetKeywordTable(AvovNames);

	//default to all on.
	relationshipFromOperands = (TranslateComplex | Rotate) & ~RotateZ;
	relationshipToResult = ComplexMask & ~RotateZ;

	//default to invisible.
	visible = GL_FALSE;

	//empty the min/max info.
	distance = 0;
	updatingScale = averageMean = GL_FALSE;
	for (i = X; i <= Z; i++)
		minPose[i] = maxPose[i] = NULL;
}

/*---------------------------------------------------------------------------*/
AveragingOverview::~AveragingOverview()
{
}

/*---------------------------Interaction Facilitation------------------------*/
/*---------------------------------------------------------------------------*/
void AveragingOverview::PoseChanged(PosableStateBase *whichPose,
									long whatChanged)
{
	GLfloat accumPos[3] = {0, 0, 0}, accumRot[3] = {0, 0, 0};
	int i;
	GLboolean doTranslate, doRotate;

	if ((whatChanged & ChangeCurrentMask) == 0) //only care about "current"
		return;

	//update our averages.
	if (whichPose != pose)
	{

		if ((whatChanged & relationshipToResult
			 & (Translate | Rotate)) == NoRelationship)
			doTranslate = doRotate = GL_FALSE;
		else
		{
			whatChanged &= relationshipToResult;
			doTranslate = (whatChanged & Translate)? GL_TRUE : GL_FALSE;
			doRotate = (whatChanged & Rotate)? GL_TRUE : GL_FALSE;
		}
		for (i = 0; i < numOperands; i++)
		{
			if (doTranslate && averageMean)
			{
                            gz4d::Point<double> current = poseOfOperands[i]->getPosition();
				//cheaper just to do it than to check individual parts.
				accumPos[X] += current[X];
				accumPos[Y] += current[Y];
				accumPos[Z] += current[Z];
			}
			if (doRotate)
			{
				gz4d::Point<double> current = poseOfOperands[i]->getOrientationAxis();
				//cheaper just to do it than to check individual parts.
				accumRot[X] += current[X];
				accumRot[Y] += current[Y];
			}
		}
		pose->AccumulateNotifications();
		if (doTranslate)
		{
			pose->setPosition(accumPos[X] / numOperands,
							  accumPos[Y] / numOperands,
							  accumPos[Z] / numOperands);
		}
		if (doRotate)
			pose->setOrientation(accumRot[X] / numOperands,
								 accumRot[Y] / numOperands);
	}

	//go ahead and update the scale in a separate procedure.
	if ((whatChanged & ChangeTranslateMask) &&
		(relationshipToResult & ScaleComplex) == ScaleComplex)
		updateScale(whichPose);

	if (whichPose == pose)
		Object::PoseChanged(whichPose, whatChanged);
	else
		pose->ResumeSingularNotification();
}

/*---------------------------------------------------------------------------*/
void AveragingOverview::updateScale(PosableStateBase *changedPose,
									GLboolean leaving)
{
	int i, j;
        gz4d::Point<double> pos = gz4d::Point<double>::Invalid();
	GLfloat val, newVal;
	PosableStateBase *winner;
	GLboolean includeTarget;

	if (updatingScale)
		return;
	updatingScale = GL_TRUE;

	if (!leaving)
		pos = changedPose->getPosition();

	includeTarget = !(relationshipToResult & Translate);

	for (i = X; i <= Z; i++)
	{
		//---check the min---
		if (minPose[i] == NULL) //no info yet.  Fill in if not leaving.
		{
			if (leaving)
				continue;
			minPose[i] = changedPose;
			minPos[i] = pos[i];
		}
		else if (minPose[i] == changedPose) //was min already.  Still min?
		{
			if (minPos[i] > pos[i])
				minPos[i] = pos[i];
			else if (numOperands > 0 || includeTarget)// Need to check everyone!
			{
				if (includeTarget)
				{
					winner = pose;
					j = 0;
				}
				else
				{
					winner = poseOfOperands[0];
					j = 1;
				}
				val = winner->getPosition()[i];
				for (; j < numOperands; j++)
				{
					newVal = poseOfOperands[j]->getPosition()[i];
					if (val > newVal)
					{
						winner = poseOfOperands[j];
						val = newVal;
					}
				}
				minPose[i] = winner;
				minPos[i] = val;
			}
			else
				minPose[i] = NULL;
		}
		else if (!leaving && (minPos[i] > pos[i]))
		{
			minPose[i] = changedPose;
			minPos[i] = pos[i];
		}

		//---check the max---
		if (maxPose[i] == NULL) //no info yet.  Fill in if not leaving.
		{
			if (leaving)
				continue;
			maxPose[i] = changedPose;
			maxPos[i] = pos[i];
		}
		else if (maxPose[i] == changedPose) //was max already.  Still max?
		{
			if (maxPos[i] < pos[i])
				maxPos[i] = pos[i];
			else if (numOperands > 0 || includeTarget)// Need to check everyone!
			{
				if (includeTarget)
				{
					winner = pose;
					j = 0;
				}
				else
				{
					winner = poseOfOperands[0];
					j = 1;
				}
				val = winner->getPosition()[i];
				for (; j < numOperands; j++)
				{
					newVal = poseOfOperands[j]->getPosition()[i];
					if (val < newVal)
					{
						winner = poseOfOperands[j];
						val = newVal;
					}
				}
				maxPose[i] = winner;
				maxPos[i] = val;
			}
			else
				maxPose[i] = NULL;
		}
		else if (!leaving && (maxPos[i] < pos[i]))
		{
			maxPose[i] = changedPose;
			maxPos[i] = pos[i];
		}
	}
	//find the diagonal of the encompassing cube.
	distance = 0.0;
	if (!averageMean)
	{
		pose->setPosition((maxPos[X] + minPos[X])/2,
						  (maxPos[Y] + minPos[Y])/2,
						  (maxPos[Z] + minPos[Z])/2);
	}
	pos = pose->getPosition();
	for (i = X; i <= Z ; i++)
	{
		newVal = maxPos[i] - pos[i];
		if (newVal > distance)
			distance = newVal;
		newVal = pos[i] - minPos[i];
		if (newVal > distance)
			distance = newVal;
	}
	if (distance < 1.0f) //limit how small we can get
		distance = 1.0f;

	pose->setScale((float)((distance /* * SQRT3 */) / ViewManager::radiusOfWorkspaceCenter));
	updatingScale = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
long AveragingOverview::manage(PosableStateBase *pose)
{
	long retval = CompositeObject::manage(pose);

	if (relationshipToResult & ScaleComplex)
		updateScale(pose);

	return retval;
}

/*---------------------------------------------------------------------------*/
void AveragingOverview::relinquish(PosableStateBase *pose)
{
	CompositeObject::relinquish(pose);

	if (relationshipToResult & ScaleComplex)
		updateScale(pose, GL_TRUE);
}

/*---------------------------------------------------------------------------*/
/**	This procedure allows the caller to reinstate a disabled feature.  The
 *	passed changeMask indicates what in the result can be affected by
 *	the operands.
 */
void AveragingOverview::enable(long changeMask)
{
	//First, get rid of unwanted bits
	changeMask &= ComplexMask;
	changeMask &= ~RotateZ;

	relationshipToResult |= changeMask;
	relationshipFromOperands |= (changeMask & ~Scale);

	//ensure that proper bits are set if Scale bits are active
	if (relationshipToResult & Scale)
	{
		relationshipFromOperands |= TranslateComplex;
		relationshipToResult |= ScaleComplex;
	}
}

/*---------------------------------------------------------------------------*/
/**	This procedure allows the caller to disabled a feature.  The
 *	passed changeMask indicates what in the result will no longer
 *	be affected by the operands.
 */
void AveragingOverview::disable(long changeMask)
{
	//First, get rid of unwanted bits
	changeMask &= ComplexMask;

	relationshipToResult &= ~changeMask;
	relationshipFromOperands &= ~changeMask;

	//ensure that proper bits are set if Scale bits are active
	if (relationshipToResult & Scale)
	{
		relationshipFromOperands |= TranslateComplex;
		relationshipToResult |= ScaleComplex;
	}
}

/*-----------------------------Persistence-----------------------------------*/
/*---------------------------------------------------------------------------*/
int AveragingOverview::processField(int foption, char **fields)
{
	const char *option;
	int i = 1;
	
	switch(foption)
	{
		case TRACKING:
			option = fields[i++];
			relationshipToResult = NoRelationship;
			while (*option != '\0')
			{
				switch (*option)
				{
					case 'H':
						relationshipToResult |= RotateY;
						break;
					case 'P':
						relationshipToResult |= RotateX;
						break;
					case 'R':
						relationshipToResult |= RotateZ;
						break;
					case 'S':
						relationshipToResult |= ScaleComplex;
						break;
					case 'T':
						relationshipToResult |= Translate;
						break;
					case 'X':
						relationshipToResult |= TranslateX;
						break;
					case 'Y':
						relationshipToResult |= TranslateY;
						break;
					case 'Z':
						relationshipToResult |= TranslateZ;
						break;
				}
				option++;
			}
			break;
		case AVERAGE:
			if (strcmp(fields[i++], "MEAN") == 0)
				averageMean = GL_TRUE;
			else
				averageMean = GL_FALSE; //MIN_MAX
			break;
		default:
			return CompositeObject::processField(foption, fields);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
int AveragingOverview::writeTo(FILE *file)
{
	fprintf(file, "%s %d\n", AvovNames[N_RECORDS], numOperands);
	return CompositeObject::writeTo(file);
}

/*---------------------------Rendering/Info----------------------------------*/
/*---------------------------------------------------------------------------*/
void AveragingOverview::DrawLowRes(float degradation, Window3D *win)
{
//	int i;
	GLfloat dist/*, vals[2] = {1.0, distance}*/;

	glColor3f(1.0, 1.0, 1.0);
	glNormal3f(0.0, 0.0, 1.0);
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
/*	for (i = 0; i < 2; i++)
	{
		dist = vals[i];
*/		dist = 0.5f;
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
//	}
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LIGHTING);
}

/*---------------------------------------------------------------------------*/
int AveragingOverview::DrawHighRes(int eye, Window3D *win)
{
	DrawLowRes(0.0f, win);
	return 1;
}
