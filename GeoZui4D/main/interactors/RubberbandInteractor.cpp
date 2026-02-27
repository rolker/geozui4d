/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// RubberbandInteractor.cpp
/** 
 * Interactor for handling input for rubberbands (for GUTMs only at this point).
 *
 *
 * Created by mdp 10/18/2004
 */
///////////////////////////////////////////////////////////////////////////////

#include "interactors/RubberbandInteractor.h"
#include "objects/gutm.h"
#include "engine/InputMediator.h"

RubberbandInteractor *gutmRubberband;

/*---------------------------------------------------------------------------*/
RubberbandInteractor::RubberbandInteractor()
:Interactor("gutmRubberband")
{
	targetGutm = NULL;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int RubberbandInteractor::HandleInput(unsigned int inputType, InputInfo *info)
{
	char val;
	GLboolean cntrlPressed;

	if (inputType & EnumeratedAction)
		return PASS_EVENT_THROUGH;

	if (targetGutm && (inputType & KeyStroke))
    {
		val = info->val.i;
		cntrlPressed = (info->modifier.i & KeyLeftControl) || (info->modifier.i & KeyRightControl);
		if (cntrlPressed)
			val += 96;
		if (val == 'c' || val == 'h' || val == 'r')
		{
			if (val == 'c')
				targetGutm->rubberbandCopy();
			else if (val == 'h')
				targetGutm->rubberbandHole();
			else if (val == 'r')
				targetGutm->rubberbandRestore();
			if (!cntrlPressed)
			{
				attachToGutm(NULL);
				return REMOVE_FROM_STREAM;
			}
		}
	}
	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
void RubberbandInteractor::RemovalNotify(InputMediator *fromList)
{
	if (targetGutm)
		targetGutm->setRubberband(NULL);
	rubberband.SetInvisible();
}

/*---------------------------------------------------------------------------*/
void RubberbandInteractor::attachToGutm(Gutm *g)
{
	Gutm *oldGutm = targetGutm;

	if (targetGutm)
		targetGutm->setRubberband(NULL);
	targetGutm = g;
	if (targetGutm)
	{
		targetGutm->setRubberband(&rubberband);
		if (!oldGutm)
			globalInputMediator->PushInteractor(this);
	}
}

