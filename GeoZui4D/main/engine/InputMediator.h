/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// InputMediator.h


#ifndef INPUTMEDIATOR_H
#define INPUTMEDIATOR_H

#include "engine/Interactor.h"
#include "widgets/Sv_Standards.h"

#define INPUT_NON_ID -1
/**
 * This class mediates control over input signals among interactors.
 *
 * \author Matt Plumlee (mdp)
 * \date 3/2/2000
 * \image html EventIllustration.jpg
 */
///////////////////////////////////////////////////////////////////////////////
class InputMediator : public Interactor
{   public:
		InputMediator(int retval = SINK_EVENT);
                InputMediator(std::string name, int retval = SINK_EVENT);
		~InputMediator();

		virtual int PushInteractor(Interactor *client, int instanceInfoOrID = INPUT_NON_ID);
		virtual int PopInteractor(Interactor *client = NULL /* For client-checking, if desired */,
		                  int instanceInfoOrID = INPUT_NON_ID); /* (fails if not top item) */

		virtual int BringInteractorToTop(Interactor *client, int instanceInfoOrID = INPUT_NON_ID);
		virtual int RemoveInteractor(Interactor *client, int instanceInfoOrID = INPUT_NON_ID);

		int HandleInput(unsigned int inputType, InputInfo *info);
		void Trigger(unsigned int eventType, void *data = NULL, void *modifier = NULL);

		InputMediator *getPriorityMediator();

		/*---Scripting funcitons---*/
		virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
		virtual int list(Tcl_Interp *interp);

		GLboolean isPermanent;

	protected:
		void init(int retval);
		int packArgument(void *&result, const char *flag, const char *value, Tcl_Interp *interp);

		struct InteractorList //conceptually a stack, but manipulable like a list
		{   InteractorList(Interactor *i, int instanceInfoOrID = INPUT_NON_ID)
				{client = i; instanceInfo = instanceInfoOrID; prev = next = NULL;}
		
		    Interactor *client;
			int instanceInfo;
			InteractorList *prev, *next;
		};

		InteractorList *findInteractor(Interactor *i, int instanceInfoOrID = INPUT_NON_ID);
		int removeListMember(InteractorList *listMember, GLboolean notifyMember = GL_TRUE);
		InteractorList *interactors;

		InputMediator *priorityMediator;

		virtual DottedAddress *getClientAddress(Interactor *i);
};

extern InputMediator *globalInputMediator,
					 *highLevelEventMediator,
					 *interactorEventMediator;

extern DottedAddress *EventMediatorNeighborhood;

#endif

