/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// EventHandler.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class EventHandler; //this line connects the above comments with this class.

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "engine/Interactor.h"
/**
 *	This class implements a handler for dispatching events through scripts
 *	and procedures, useful for scripts and non-Interactors.
 *	This class implements the following responsibilities:
 *	\li	Execute a script or procedure when a selected event is encountered.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 8/21/2002
 */
class EventHandler : public Interactor
{
   public:
		//Initialization
       EventHandler(std::string name);
		~EventHandler(void);

		//Interaction functions
		int HandleInput(unsigned int inputType, InputInfo *info);

		//Setup functions
		void setCallback(int (*voidProc)(), unsigned int inputType);
		void setCallback(int (*stdProc)(unsigned int, InputInfo *));
		void setScript(char *script, unsigned int inputType);
		void setScript(char *script);
		void setSink(unsigned int inputType);

	protected:
		void fillCallback(char *&resultScript, char *baseScript, InputInfo *info);
                void append(char *&resultScript, std::string sval, int &i, int &resultAlloc);
		void append(char *&resultScript, long int val, int &i);
		void append(char *&resultScript, InputInfo::polyInfo &info, int &i, int &resultAlloc, int type);
		void scanForPercent();

		enum {Empty, VoidProcedure, StandardProcedure, SpecificScript, GeneralScript, Sink};
		int type;
		union
		{
			int (*voidProc)();
			int (*stdProc)(unsigned int, InputInfo *);
			char *script;
		} handler;
		unsigned int inputToHandle;
		int hasPercent;
		bool nextIntAsChar;
};

#endif
