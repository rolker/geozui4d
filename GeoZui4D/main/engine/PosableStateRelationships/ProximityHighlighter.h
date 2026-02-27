/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// ProximityHighlighter.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class ProximityHighlighter;

#ifndef PROXIMITYHIGHLIGHTER_H
#define PROXIMITYHIGHLIGHTER_H

#include "CompositeObject.h"
/** 
 *	This class provides a way to detect close proximity among a selected
 *	set of reference frames, and lock a view on two close-proximity objects.
 *	Scale is updated such that the positions of the two nearest objects are
 *	present in the center of workspace of any ViewManager attached to this.
 *	This class implements the following responsibilities.
 *	\li Managing PosableStateBases by collecting change events
 *		and keeping the result pose updated.
 *	\li Rendering to the screen (renders a wire box, if visible)
 *	\li Loading and saving from/to disk
 *
 *	\author Matt Plumlee (mdp)
 *  \date 6/28/2001
 */
class ProximityHighlighter: public CompositeObject
{
  public:
	ProximityHighlighter(void *creator = NULL);
	virtual ~ProximityHighlighter();
	DERIVE_OBJECT(ProximityHighlighter);

	/*---File-related---*/
	virtual int readFrom(FILE *file);
	virtual int processField(int option, char **fields);
	virtual int writeTo(FILE *file);

	/*---Interaction facilitation---*/
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);
	virtual long manage(PosableStateBase *pose); //assigns operands
	virtual void relinquish(PosableStateBase *pose = NULL); //relinquish all operands if NULL
	void setThreshold(float newThreshold);
	void trackPosition(GLboolean track = GL_TRUE);
	enum alarmStates {AlarmSafe, AlarmCaution, AlarmDanger};

	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

	/*---Rendering---*/
	virtual void DrawLowRes(float degradation, Window3D *win);
	virtual int DrawHighRes(int eye, Window3D *win); //return 1 if done, 0 if not done.

	/*---Event notification---*/
	static unsigned int InProximityEvent() { return inProximityInputType->value;}
	static unsigned int InCloseProximityEvent() { return inCloseProximityInputType->value;}
	static unsigned int OutOfProximityEvent() { return outOfProximityInputType->value;}
	static unsigned int OutOfCloseProximityEvent() { return outOfCloseProximityInputType->value;}
	static unsigned int ProximityHighlightChangeEvent() { return proximityHighlightChangeInputType->value;}

  protected:
	//distance matrix
	int matrixSize;
	float *distanceToNeighbor;
	int *cellWithMin;

	//summary values
	int closestIndexRow, closestIndexColumn;
	float threshold, minDistance, scaleFactor;
	int alarmState;
	GLboolean readingFile, debugFlag;

	static const EventType *inProximityInputType;
	static const EventType *inCloseProximityInputType;
	static const EventType *outOfProximityInputType;
	static const EventType *outOfCloseProximityInputType;
	static const EventType *proximityHighlightChangeInputType;

	enum ProximityParams {THRESHOLD = FirstChildKeyword, TRACKING};

	void updateMinTable();
	void updateMinTable(int index);
	void updatePose();
	void soundAlarm();
	void cancelAlarm();
};

#endif
