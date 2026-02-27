/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// WindowMediator.h

///////////////////////////////////////////////////////////////////////////////
class WindowMediator; //this line connects the above comments with this class.

#ifndef WINDOWMEDIATOR_H
#define WINDOWMEDIATOR_H

#include "engine/InputMediator.h"
#include "engine/ToglWindow.h"
/**
 *  This class mediates control over idle events among ToglWindows, sending
 *	and every idle event to each registered ToglWindow.  All other events
 *	are distributed to the window indicated in the event structure.
 *
 * \author Matt Plumlee (mdp)
 * \date 7/25/2000
 */
class WindowMediator : public InputMediator
{   public:
        WindowMediator(std::string name);

		/*---Client management functions: these force the client to be a ToglWindow---*/
		int PushWindow(ToglWindow *client, int instanceInfoOrID = INPUT_NON_ID);
		int PopWindow(ToglWindow *client = NULL /* For client-checking, if desired */,
		                  int instanceInfoOrID = INPUT_NON_ID) /* (fails if not top item) */
			{return InputMediator::PopInteractor(client, instanceInfoOrID);}

		int BringWindowToTop(ToglWindow *client, int instanceInfoOrID = INPUT_NON_ID)
			{return InputMediator::BringInteractorToTop(client, instanceInfoOrID);}
		int RemoveWindow(ToglWindow *client, int instanceInfoOrID = INPUT_NON_ID)
			{return InputMediator::RemoveInteractor(client, instanceInfoOrID);}

		/*---Input handler---*/
		int HandleInput(unsigned int inputType, InputInfo *info);

	protected:
		virtual DottedAddress *getClientAddress(Interactor *i);

	private:
		/*---Kill all the general interactor functions---*/
		int PushInteractor(Interactor *client, int instanceInfoOrID) {return 0;}
		int PopInteractor(Interactor *client, int instanceInfoOrID) {return 0;}
		int BringInteractorToTop(Interactor *client, int instanceInfoOrID) {return 0;}
		int RemoveInteractor(Interactor *client, int instanceInfoOrID) {return 0;}
};

extern WindowMediator *windowMediator;

#endif

