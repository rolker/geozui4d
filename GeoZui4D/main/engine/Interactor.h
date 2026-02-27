/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// Interactor.h

class Interactor;

#ifndef INTERACTOR_H
#define INTERACTOR_H

#include "engine/InputInfo.h"
#include "utilities/Addressable.h"

struct Tcl_Interp;
class InputMediator;


class EventType: public Addressable
{
	public:
            EventType(std::string name);
		unsigned int value;
		//unsigned char permanentEvent;
		bool permanentEvent;
};

/**
 * This is the parent class for the input handling of events over which it is
 * given authority.<br>
 * It has the following responsibilities:
 *	\li	Process input events
 *	\li	Recognize a "cancel" event
 *	\li	Provide unique identifiers for new event types (static)
 *
 *  \author Matt Plumlee (mdp)
 *  \date 3/2/2000
 *  \image html interactorSubsystem.jpg
 *  \image html interactorHierarchy.jpg
 *  \image html interactorParadigm.jpg
 *  \image html inputFlowThroughInteractors.jpg
 */
///////////////////////////////////////////////////////////////////////////////
class Interactor: public Addressable
{   public:
        enum EventReturnAction
        {
            PASS_EVENT_THROUGH = 0,  // return values used
            SINK_EVENT         = 1,  // by HandleInput()
            REMOVE_FROM_STREAM = -1 // method, if desired.
        };

		enum InputType
		{
			NoInput         = 0x0000,
			InvalidInput	= 0x0007,

			DataAsInt       = 0x0001,
			DataAsLong      = 0x0002,
			DataAsDouble    = 0x0003,
			DataAsAddress   = 0x0004,
			DataAsString    = 0x0005,
			DataAsPointer   = 0x0006,
			DataTypeMask    = 0x0007,

			CoordinatesHaveZ= 0x0008,

			KeyStroke       = 0x0010,
			KeyDown	        = 0x0010, //KeyDown and KeyUp added by
			KeyUp           = 0x0020, //mdp 7/18/2000 to allow detection
			KeyAction       = 0x0030,

			MouseButtonBase = 0x0020,
			MouseButton1    = 0x0040,
			MouseButton2    = 0x0080,
			MouseButton3    = 0x0100,
			MouseButtons    = 0x01C0,

			MouseMotion     = 0x0200,
			MouseEnterMask  = 0x0400,
			MouseEnter      = 0x0600,
			MouseLeaveMask  = 0x0800,
			MouseLeave      = 0x0A00,
			MouseButtonDown = 0x1000,
			MouseButtonUp   = 0x2000,
			MouseState      = 0x3000,
			MouseDouble     = 0x4000,
			MouseAction     = 0x7FC0,

			BasicActionMask = 0x7FF0,
			StreamSpecific	= 0x80000000,
			StreamSpecificMask = 0x7FFF0000,

			EnumeratedAction= 0x8000, //when we run out of bits, give us 2K more.
			EnumIncrement   = 0x0010,
			EnumBits        = 4,
			BeginIdleAction = 0x8010,
			EndIdleAction   = 0x8026,
			BreakAction     = 0x8030, //Break causes any in-progress interaction to cease (mdp 8/21/2000)
			NextEnumAction  = 0x8040,
		    EnumeratedMask  = 0x7FF0,

			AnyAction       = 0xFFFFFFF0

			//Note: top 2 bytes can be used for event type
			//      definitions for stream-specific events.
			//		The StreamSpecific bit must be set for these.
		};

		enum ModifierType //Added by mdp 7/18/2000
		{
			ModNone	        = 0x00000,
			ModShift        = 0x00001,
			ModCapsLock     = 0x00002,
			ModControl      = 0x00004,
			ModNumLock      = 0x00008,
			ModScrollLock   = 0x00020,
			ModAlt          = 0x20000
		};

		enum SpecialKey //Added by mdp 7/25/2000
		{
			KeyNone			= 0xFF00, //I just made this up (mdp 8/25/2000)
			KeySpecial		= 0xFF00, //mask indicating specialness (mdp 8/25/2000)

			KeyLeftShift    = 0xFFE1,
			KeyRightShift   = 0xFFE2, //On my keyboard, this is only for downstrokes.
			KeyLeftControl  = 0xFFE3,
			KeyRightControl = 0xFFE4, //On my keyboard, this is only for downstrokes.
			KeyCapsLock     = 0xFFE5,
			KeyLeftAlt      = 0xFFE9,
			KeyRightAlt     = 0xFFEA, //On my keyboard, this is only for downstrokes.

			KeyBackspace    = 0xFF08,
			KeyEnter		= 0xFF0D,

			//On my keyboard, up and down are generated together for the next 3.
			KeyTab			= 0xFF09,
			KeyPause        = 0xFF13,
			KeyBreak        = 0xFF69,
			
			KeyScrollLock   = 0xFF14,
			KeyEsc          = 0xFF1B,

			KeyFBase        = 0xFFBD, //Add the function key number to this (F1 = 0xFFBE)

			KeyHome         = 0xFF50,
			KeyPageUp       = 0xFF55,
			KeyPageDown     = 0xFF56,
			KeyEnd          = 0xFF57,
			KeyInsert       = 0xFF63,
			KeyDelete       = 0xFFFF,

			KeyLeftArrow    = 0xFF51,
			KeyUpArrow      = 0xFF52,
			KeyRightArrow   = 0xFF53,
			KeyDownArrow    = 0xFF54,
			KeyNoArrow      = 0xFF0B, //The 5 key on the number pad with NumLock off.

			KeyWinStartMenu = 0xFF5C,
			KeyWinMenu      = 0xFF5D
		};

		virtual int HandleInput(unsigned int inputType, InputInfo *info);
		virtual void RemovalNotify(InputMediator *fromList) {};
		Interactor(int retval = SINK_EVENT);
                Interactor(std::string name, int retval = SINK_EVENT);
        virtual ~Interactor();
        virtual std::string nameOfEventType(unsigned int inputType);
        virtual unsigned int valueOfSpecificEventType(std::string eventType);

		static EventType *newEventType(const char *name = NULL, unsigned int flags = 0);
		static unsigned int interactorDestroyedEvent() { return interactorDestroyedInputType->value;}

		/*---Scripting funcitons---*/
		//virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
		//virtual int list(Tcl_Interp *interp);

	protected:
		virtual int IsCancelAction(int inputType, InputInfo *info);

        bool isMouseButton1Pressed( int inputType );
        bool isMouseButton1Released( int inputType );
        bool isMouseButton2Pressed( int inputType );
        bool isMouseButton2Released( int inputType );
        bool isMouseButton3Pressed( int inputType );
        bool isMouseButton3Released( int inputType );

		static const EventType *interactorDestroyedInputType;

	private:
		static unsigned int nextEnumeratedAction;
		int	defaultReturn;

		static EventType **globalEvents;
		static int numGlobalEventsAllocated;
};

extern Interactor *defaultInteractor;
extern DottedAddress *InteractorNeighborhood, *EventNeighborhood, *UserHandlerNeighborhood;

#endif
