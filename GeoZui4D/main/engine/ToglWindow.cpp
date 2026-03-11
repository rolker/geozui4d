// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// ToglWindow.cpp
// 
// This is the abstract class that encapsulates a 3-D OpenGL window with Togl.
//
// Created by mdp 7/14/2000
///////////////////////////////////////////////////////////////////////////////

//#ifdef _DEBUG
//#define BUILD_wrapper
//#include "proWrap.h"
//#define STARTING_SCRIPT "./debug.tcl"
//#else
#define STARTING_SCRIPT "./visualizer.tcl"
//#endif

#ifdef MSVC
//#ifndef __APPLE__
#include "tkTable.h"
#undef EXTERN
#undef FAR
#undef max
#undef min
//#endif // __APPLE__
#endif

#include "engine/ToglWindow.h"
#include "MainWindow.h"

//#include <stdlib.h>

#include <typeinfo>

#include "SDV_types.h"
#include "engine/SessionRecorder.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/gzCommandInterface.h"
#include "interactors/PlaybackTool.h"
#include "utilities/DottedAddress.h"
#include "interactors/SDV_Cursor.h"
/*
#include "itcl.h"
#include "itk.h"
*/
#include <string.h>


#ifndef WIN32
	extern pid_t pid;
	#define GETPID getpid
#else
	#include<process.h>
	extern int pid;
	#define GETPID _getpid
#endif


#define MAXCOMMAND 256
//#define FONT GLUT_BITMAP_9_BY_15
//#define FONT GLUT_BITMAP_8_BY_13
//#define FONT GLUT_BITMAP_TIMES_ROMAN_10
//#define FONT GLUT_BITMAP_TIMES_ROMAN_24
//#define FONT GLUT_BITMAP_HELVETICA_10
//#define FONT GLUT_BITMAP_HELVETICA_12
//#define FONT GLUT_BITMAP_HELVETICA_18

Tcl_Interp *  ToglWindow::interp = NULL;
GLboolean ToglWindow::toglIsValid = GL_FALSE;
std::string ToglWindow::setupScript = "";
extern gzCommandInterface *gz;


/*---------------------------------------------------------------------------*/
int dominantEye = LEFT_EYE;  // Uses LEFT_EYE or RIGHT_EYE as defined in SDV_types.h

int ToglWindow::fishtankVR_flag = -1;	//fishtank VR enabled, but not activated
//#ifdef USING_HEADTRACKING
#ifndef _DEBUG
int ToglWindow::headtracking_flag = -1;  //headtracking enabled, but not activated
//int ToglWindow::headtracking_flag = 0;  //temp disable on Release version
#else
int ToglWindow::headtracking_flag = 0;   //disabled for debug as Isotrack CreateThread() method
#endif										 // does not currently work except in release version
//#else
//int ToglWindow::headtracking_flag = 0;  //headtracking disabled
//#endif
//#endif

int ToglWindow::stereo_flag = -1;  //stereo disabled by default
int ToglWindow::ssstereo_flag = -1;  //side by side stereo disabled by default
int ToglWindow::oneEyedFlag = 0;  //stereo disabled by default
char * ToglWindow::datadir = NULL;
char * ToglWindow::appdir = NULL;

int ToglWindow::masterPort = 0;  // if != 0, I am master and brodcast on this port
char * ToglWindow::masterAddress = NULL; // address master broadcasts on 

int ToglWindow::slavePort = 0; // if != 0, I am slave and listen on this port

int ToglWindow::ioPort = 1974; // if != 0, send tcl cmds to this port

SDV_Cursor *ToglWindow::sdvcursor = NULL;
int ToglWindow::tkInitialized = 0;
ToglWindow *ToglWindow::creatingWindow = NULL;
ToglWindow *ToglWindow::pointerForRetrieval = NULL;
long ToglWindow::idleEventCount = 0;
float ToglWindow::bgColor[4] = {0.0, 0.0, 0.0, 1.0};

int ToglWindow::screenWidth_pix = 1600; // default, can be modified in .conf file
int ToglWindow::screenHeight_pix = 1200;

double ToglWindow::screenLeft = -0.20;
double ToglWindow::screenRight = 0.20;
double ToglWindow::screenTop = 0.15;
double ToglWindow::screenBottom = -0.15;
double ToglWindow::eyeX = 0.0;
double ToglWindow::eyeY = 0.0;
double ToglWindow::eyeZ = 0.5;

int ToglWindow::mainRight = 0;

int ToglWindow::disableRTVideo = 1;

Addressable InteractorWindowNeighborhoodAnchor;
DottedAddress *InteractorWindowNeighborhood = new DottedAddress(&InteractorWindowNeighborhoodAnchor, "Window");

/*-----------------------Constructors/Destructors----------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	Constructor.  Initializes the object bookkeeping and input mediation.
 */
ToglWindow::ToglWindow(std::string name)
:Interactor(name),
 inputMediator(name)
{
	width = 400;
	height = 300;
	xOffset = yOffset = 0;
	myTogl = NULL;

	creatingWindow = this;
	//set up interactors
	inputMediator.PushInteractor(this);
	if (!name.empty())
		getAddress()->reparent(InteractorWindowNeighborhood);
}

/*---------------------------------------------------------------------------*/
/**
 *	Allows for child-specific initialization, sets up the Tcl interpreter if
 *	it is not already set up.  All child classes must call the parent's Init
 *	function at the top of their implementation of this procedure.
 */
void ToglWindow::Init(int argc, char *argv[])
{
	if (!tkInitialized)
	{
		tkInitialized = 1;
//#ifdef _DEBUG
//		TclPro_Init(&argc, &argv);
//#endif
		//Tk_Main(argc,argv,InitTkCallBack ); //infinite loop.
                Tk_Main(1,argv,InitTkCallBack ); //infinite loop.
		return;
	}
	interp = Togl_Interp(myTogl);
	
}

/*---------------------------------------------------------------------------*/
/**
 *	Returns the Togl object associated with this ToglWindow.
 */
Togl *ToglWindow::GetTogl()
{
	return myTogl;
}

/*---------------------------------------------------------------------------*/
/**
 *	Initializes Tcl/Tk and Togl commands.  Should be called only once per
 *  Tcl interpreter.
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ToglWindow::InitTkCallBack(Tcl_Interp *interp)
{
	char const *newCommand[] = {"keyboard", "mouse", "motion",
						  "idleCallBack", "bgcolor", "setPointerForRetrieval",
						  "recordCommand", "recordTimestamp", "recorder",
						  "recordBookmark", "caption",
						  NULL};
	int currentCommand;
   /*
    * Initialize Tcl, Tk, and the Togl widget module.
    */
	if (Tcl_Init(interp) == TCL_ERROR)
	{
		return TCL_ERROR;
	}
	if (Tk_Init(interp) == TCL_ERROR)
	{
		return TCL_ERROR;
	}
	/*--could use these in the future--
	if (Itcl_Init(interp) == TCL_ERROR) {
		return TCL_ERROR;
	}
	if (Itk_Init(interp) == TCL_ERROR)
	{
		return TCL_ERROR;
	}
 */
	if (Togl_Init(interp) == TCL_ERROR)
	{
		return TCL_ERROR;
	}

#ifdef MSVC
//#ifndef __APPLE__
	if (Tktable_Init(interp) == TCL_ERROR)
	{
		return TCL_ERROR;
	}
//#endif // __APPLE__
#endif

	if (gz->Gz_Init(interp) == TCL_ERROR)
	{
		return TCL_ERROR;
	}

   /*
    * Specify the C callback functions for widget creation, display,
    * and reshape.
    */
	Togl_CreateFunc( InitToglCallBack );
	Togl_DestroyFunc( ZapCallBack );
	Togl_DisplayFunc( DisplayCallBack );
	Togl_ReshapeFunc( ReshapeCallBack );
	for (currentCommand = 0; newCommand[currentCommand]; currentCommand++)
		Togl_CreateCommand(newCommand[currentCommand], SubCommandCallBack);

	::Tcl_LinkVar(interp, "fishtankVRflag", (char *)&fishtankVR_flag, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "headtrackingflag", (char *)&headtracking_flag, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "stereoflag", (char *)&stereo_flag, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "ssstereoflag", (char *)&ssstereo_flag, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "dominantEye", (char *)&dominantEye, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "oneEyedFlag", (char *)&oneEyedFlag, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "datadir", (char *)&datadir, TCL_LINK_STRING);
	::Tcl_LinkVar(interp, "appdir", (char *)&appdir, TCL_LINK_STRING);
	::Tcl_LinkVar(interp, "masterPort", (char *)&masterPort, TCL_LINK_INT);
        ::Tcl_LinkVar(interp, "masterAddress", (char *)&masterAddress, TCL_LINK_STRING);
        ::Tcl_LinkVar(interp, "slavePort", (char *)&slavePort, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "ioPort", (char *)&ioPort, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "screenWidth_pix",(char *)&screenWidth_pix, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "screenHeight_pix",(char *)&screenHeight_pix, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "screenLeft", (char *)&screenLeft, TCL_LINK_DOUBLE);
	::Tcl_LinkVar(interp, "screenRight", (char *)&screenRight, TCL_LINK_DOUBLE);
	::Tcl_LinkVar(interp, "screenTop", (char *)&screenTop, TCL_LINK_DOUBLE);
	::Tcl_LinkVar(interp, "screenBottom", (char *)&screenBottom, TCL_LINK_DOUBLE);
	::Tcl_LinkVar(interp, "eyeX", (char *)&eyeX, TCL_LINK_DOUBLE);
	::Tcl_LinkVar(interp, "eyeY", (char *)&eyeY, TCL_LINK_DOUBLE);
	::Tcl_LinkVar(interp, "eyeZ", (char *)&eyeZ, TCL_LINK_DOUBLE);
	::Tcl_LinkVar(interp, "mainRight", (char *)&mainRight, TCL_LINK_INT);
	::Tcl_LinkVar(interp, "disableRTVideo", (char *)&disableRTVideo, TCL_LINK_INT);


	Tcl_SetVar( interp, "tcl_rcFileName", STARTING_SCRIPT, TCL_GLOBAL_ONLY );

    // Here, the GeoZui4D.conf file is located, and parsed to get app and data dirs.
	::Tcl_Eval(interp, setupScript.c_str());

#ifdef USE_SOUND
#ifdef WIN32
#ifdef _DEBUG
	::Tcl_Eval(interp, "load libsnackd.dll snack");
#else
	::Tcl_Eval(interp, "load libsnack.dll");
#endif
#else
	//::Tcl_Eval(interp, "load libsnack.so");
#endif
#endif

//cerr << "mainRight: " << mainRight << endl;

	//let child know we're done with this stage
	creatingWindow->initTkCallBack(interp);

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	See InitTogl.
 */
void ToglWindow::InitToglCallBack(struct Togl *togl)
{
	if (creatingWindow)
	{	creatingWindow->InitTogl(togl);
		creatingWindow = NULL;
	}
}

/*---------------------------------------------------------------------------*/
/**
 *	Initializes ToglWindow to be logically attached to the passed Togl.
 *  Calls the virtual Init function to allow class to finish initializing
 *  itself now that the Togl window is ready.
 */
void ToglWindow::InitTogl( struct Togl *togl )
{
	SetTogl(togl);                  //Set as the current GL window
	myTogl = togl;                  //We know who our togl is
	Togl_SetClientData(togl, this); //Togl struct knows who we are

	Init();
}

/*---------------------------Context Validation------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	Returns whether or not the drawing context is currently valid.
 */
GLboolean ToglWindow::contextIsValid()
{
	return toglIsValid;
}

/*---------------------------------------------------------------------------*/
/**
 *	Returns whether or not the drawing context is currently valid after
 *	trying to establish the right context.
 */
GLboolean ToglWindow::establishValidContext()
{
    
	if (!this)
		return GL_FALSE;

    if (!isBaseThread())
		return GL_FALSE;        

	SetTogl(myTogl);
	return contextIsValid();
}

/*---------------------------------------------------------------------------*/
/**
 *	Returns whether or not we are in the inital GeoZui thread. Useful to know
 *	because OpenGL call don't work well in other threads.
 */
GLboolean ToglWindow::isBaseThread()
{
    if (GETPID()!=pid && (MainWindow::getInterfaceType() != GeoCoastPilot))
        return GL_FALSE;
    return GL_TRUE;
}

/*----------------------------Input Callbacks--------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	See Zap.
 */
void ToglWindow::ZapCallBack( struct Togl *togl )
{
	ToglWindow *self = (ToglWindow*) Togl_GetClientData(togl);

	if (self)
		self->Zap();
}

/*---------------------------------------------------------------------------*/
/**
 *	This function is called when the associated Togl widget has been destroyed.
 */
void ToglWindow::Zap()
{
	delete this;
}

/*---------------------------------------------------------------------------*/
/**
 *	See Reshape.
 */
void ToglWindow::ReshapeCallBack( struct Togl *togl)
{
	ToglWindow *self = (ToglWindow*) Togl_GetClientData(togl);

	if (self)
		self->Reshape();
}

/*---------------------------------------------------------------------------*/
/**
 *	This function is called when the associated Togl widget has been resized.
 */
void ToglWindow::Reshape(int x, int y, int w, int h)
{

	if(w && h)
	{
		width = w;
		height = h;
		xOffset = x;
		yOffset = y;
	}
	else
	{
		SetTogl(myTogl);
		width = Togl_Width(myTogl);
		height = Togl_Height(myTogl);
		xOffset = x;
		yOffset = y;
	}
	
	glViewport(x, y, width, height);
};

/*---------------------------------------------------------------------------*/
/**
 *	See Display.
 */
void ToglWindow::DisplayCallBack(struct Togl *togl)
{
    ToglWindow *self = (ToglWindow*) Togl_GetClientData(togl);

    if (self)
        self->Display();
}

/*---------------------------------------------------------------------------*/
/**
 *	See SubCommand.
 *
 *	@return				TCL_OK / TCL_ERROR
 */
int ToglWindow::SubCommandCallBack(struct Togl *togl, int argc, char *argv[])
{
	ToglWindow *self = (ToglWindow*) Togl_GetClientData(togl);
	Tcl_Interp *interp = Togl_Interp(togl);
//	char shortCommand[32];

	if (self)
	{
		if (sessionRecorder->getRecordingState() == SessionRecorder::Recording)
		{
			if (strncmp(argv[1], "record", 6) && strncmp(argv[1], "idle", 4) && strncmp(argv[1], "plot", 4)
				&& strncmp(argv[1], "get", 3)
				&& (strcmp(argv[1], "mouse") || argv[5][0] != '2')
				&& (strcmp(argv[1], "posable")))
				sessionRecorder->recordCommand(argc, argv);
/*
			if (!strcmp(argv[1], "idleCallBack"))
				idleEventCount++;
			else
			{
				if (idleEventCount)
				{
					sprintf(shortCommand, "idle %ld", idleEventCount);
					sessionRecorder->recordCommand(shortCommand);
					idleEventCount = 0;
				}
				if (strcmp(argv[1], "recordCommand") != 0)
					sessionRecorder->recordCommand(argc, argv);
			}
*/
		}
		return self->SubCommand(argc, argv);
	}
	
	Tcl_AppendResult(interp, "Togl: Non-C++ widget ", argv[0], " used with\n",
							 "a C++-only subcommand",
							 NULL);
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function is the branch-point for Togl subcommands defined by
 *	ToglWindow subclasses.  All subclasses must call the parent class's
 *	SubCommand procedure after they have exhausted the subcommand
 *	possibilities in their implementation of this procedure.
 *	Under Windows, the compile flag /GR should be enabled (Settings->C++ tab,
 *	C++ Language listbox-item, "Enable Run-Time Type Information (RTTI)"
 *	check-box.  It should only every be a problem to not have /GR set
 *	when there is an error in the Tcl file.  I don't know what should be
 *	set for Solaris or Gnu compilers. (mdp)
 *
 *	@param argc Count of argv elements
 *	@param argv
 *	argv[0]		Togl object name
 *	argv[1]		Togl subcommand name (known to be registered)
 *	argv[2...]	Arguments
 *	@return				TCL_OK / TCL_ERROR
 */
int ToglWindow::SubCommand(int argc, char *argv[])
{
	Tcl_Interp *interp = Togl_Interp(myTogl);
	int val;

	if (!strcmp(argv[1], "idleCallBack"))
		return Idle(argc, argv);
	else if (!strcmp(argv[1], "keyboard"))
		return Keyboard(argc, argv);
	else if (!strcmp(argv[1], "motion"))
		return Motion(argc, argv);
	else if (!strcmp(argv[1], "mouse"))
		return Mouse(argc, argv);
	else if (!strcmp(argv[1], "bgcolor"))
		return BackgroundColor(argc, argv);
	else if (!strcmp(argv[1], "setPointerForRetrieval"))
		return PointerForRetrieval(argc, argv);
	else if (!strncmp(argv[1], "record", 6))
	{
		if (!strcmp(argv[1], "recordCommand"))
			return sessionRecorder->recordCommand(argv[2]);
		else if (!strcmp(argv[1], "recordTimestamp"))
			return sessionRecorder->recordTimestamp(argv[2]);
		else if (!strcmp(argv[1], "recordBookmark"))
		{
			if (argc >= 4)
				val = atoi(argv[3]);
			else
				val = 0;
			return sessionRecorder->recordBookmark(argv[2], val);
		}
		else if (!strcmp(argv[1], "recorder"))
		{
			if (argc < 3)
				return TCL_ERROR;
			if (!strcmp(argv[2], "file"))
				sessionRecorder->setSessionFile(argv[3]);
			else if (!strcmp(argv[2], "record"))
				return sessionRecorder->record(argv[3]);
			else if (!strcmp(argv[2], "play"))
			{
				int retval = sessionRecorder->play(argv[3]); //block until done playing.
				if (retval == TCL_OK)
					globalInputMediator->Trigger(BreakAction);
				return retval;
			}
			else if (!strcmp(argv[2], "pause"))
				return sessionRecorder->pause();
			else if (!strcmp(argv[2], "stop"))
				return sessionRecorder->stop();
			else if (!strcmp(argv[2], "time"))
			{
				char timeval[20] = "";

				sprintf(timeval, "%f", sessionRecorder->getSessionTime());
				Tcl_SetResult(interp, (char *) timeval, TCL_VOLATILE);
				return TCL_OK;
			}
			return TCL_OK;
		}
	}

	Tcl_AppendResult(interp, "Togl: Unknown option: ", argv[1], "\n",
							 argv[0], " (an instance of ",
							 typeid(*this).name(),
							 ") does not support this option.",
							 NULL);
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function causes idle-time display to occur, for instance zooming
 *  or locked-rotation with the widgets, or incremental high-res display.
 */
int ToglWindow::Idle(int argc, char *argv[])
{
	InputInfo *info = new InputInfo(this, NoInput);

	globalInputMediator->Trigger(BeginIdleAction);
	relationshipManager->flush();
	InputCallback(NoInput, info);
	delete info;
	globalInputMediator->Trigger(EndIdleAction, Togl_Interp(myTogl));

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function converts a keyboard event to an InputInfo and passes it to
 *	the InputMediator.
 *
 *	@param argc Count of argv elements
 *	@param	argv
 *	argv[2]		x-position of mouse cursor
 *	argv[3]		y-position of mouse cursor
 *	argv[4]		modifier value (shift, alt, etc.)
 *	argv[5]		keyboard character
 *	argv[6]		key state SDV_DOWN / SDV_UP
 *	argv[7]		keysym as a number
 *	@return				TCL_OK / TCL_ERROR
 */
int ToglWindow::Keyboard(int argc, char *argv[])
{
	int keysym;
	int type = (atoi(argv[6]) == SDV_UP)? KeyUp : KeyDown;

	SetTogl(myTogl);

	InputInfo *info;
	
	type += Interactor::DataAsInt;
	info = new InputInfo(this, type);

	info->x.i = atoi(argv[2]);
	info->y.i = atoi(argv[3]);
	keysym = atoi(argv[7]);
	if (keysym > 256) //if larger than a byte, must be a special character, so store it.
		info->val.i = keysym;
	else              //otherwise, just store the byte as a regular character.
		info->val.i = *argv[5];
	info->modifier.i = atoi(argv[4]);

	InputCallback(type, info);
	delete info;

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function converts a mouse motion event to an InputInfo and passes it
 *	to the InputMediator.
 *
 *	@param argc Count of argv elements
 *	@param argv
 *	argv[2]		x-position of mouse cursor
 *	argv[3]		y-position of mouse cursor
 *	argv[4]		modifier value (shift, alt, etc.)
 *	argv[5]		Motion type (-1 = leave, 1 = enter, 0 = plain)
 *	@return				TCL_OK / TCL_ERROR
 */
int ToglWindow::Motion(int argc, char *argv[] )
{
	int type = Interactor::MouseMotion + Interactor::DataAsInt;
    int motionType = atoi(argv[5]);

   	SetTogl(myTogl);

    if (motionType == -1)
        type |= MouseLeave;
    else if (motionType == 1)
        type |= MouseEnter;
    InputInfo *info = new InputInfo(this, type);
	info->x.i = atoi(argv[2]);
	info->y.i = atoi(argv[3]);
	info->modifier.i = atoi(argv[4]);
   



	InputCallback(type, info);
	delete info;
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This function converts a mouse button event to an InputInfo and passes it
 *	to the InputMediator.
 *
 *	@param argc Count of argv elements
 *	@param argv
 *	argv[2]		x-position of mouse cursor
 *	argv[3]		y-position of mouse cursor
 *	argv[4]		modifier value (shift, alt, etc.)
 *	argv[5]		Mouse button
 *	argv[6]		SDV_DOWN / SDV_UP
 *	@return				TCL_OK / TCL_ERROR
 */
int ToglWindow::Mouse(int argc, char *argv[])
{
	int type, button;

	SetTogl(myTogl);

	button = atoi(argv[5]);
	if (button >= 1 && button <= 3)
		type = (Interactor::MouseButtonBase << button);
	else
		type = 0;

	type += (Interactor::MouseButtonDown << atoi(argv[6])) + Interactor::DataAsInt;

	InputInfo *info = new InputInfo(this, type);
	info->x.i = atoi(argv[2]);
	info->y.i = atoi(argv[3]);
	info->val.i = button;
	info->modifier.i = atoi(argv[4]);
/*---able to get rid of this now that the widgets can pick twice in a row properly.
	GLint viewport[4];
	GLfloat depth;

	if(atoi(argv[6]) == SDV_DOWN)
	{
		//See if selection is on something with depth info (redraw if not)
		glGetIntegerv(GL_VIEWPORT, viewport);
		if (stereo_flag == 1) //check for stereo added by mdp 8/2/2000
			glReadBuffer(GL_FRONT_RIGHT);
		else
			glReadBuffer(GL_FRONT);
		glReadPixels(info->x.i, viewport[3] - info->y.i,1,1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		if (depth >= 1.0)
		{
//			Display();  // this might cause a problem on unix, we do it here to draw low res in case hi res has not drawn underneeth the cursor yet
		}
	}
*/
	InputCallback(type, info);
	delete info;
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	Sets the background color for the associated Togl window.
 *
 *	@param argc Count of argv elements
 *	@param	argv
 *	argv[1]		Color argument of the form \#rrggbb (hexidecimal)
 *	@return				TCL_OK / TCL_ERROR
 */
int ToglWindow::BackgroundColor(int argc, char *argv[])
{
	char *color = &(argv[2][1]);
	unsigned int value;
	float r,g,b;

	SetTogl(myTogl);

	sscanf(color, "%x", &value);
	r = (float) ((value >> 16) & 255);
	g = (float) ((value >> 8) & 255);
	b = (float) (value & 255);

	bgColor[0] = r/255.0;
	bgColor[1] = g/255.0;
	bgColor[2] = b/255.0;
	glClearColor(r/255.0, g/255.0, b/255.0, 0.0);

	return TCL_OK;
}

void ToglWindow::BackgroundColor(double r, double g, double b)
{
	bgColor[0] = r;
	bgColor[1] = g;
	bgColor[2] = b;
	glClearColor(r, g, b, 0.0);
}

/*---------------------------------------------------------------------------*/
/**
 *	Passes the structure on to the global InputMediator.
 */
int ToglWindow::InputCallback(int inputType, InputInfo *info)
{
	//Pass the event on.
	return globalInputMediator->HandleInput(inputType, info);
}

/*---------------------------------------------------------------------------*/
/**
 *	Processes events targeted for this ToglWindow.
 *
 *	@param	inputType	Type field of the info structure
 *	@param	info		Full event info structure
 *	@return				0=not handled / 1=handled / -1=handled,remove
 */
int ToglWindow::HandleInput(unsigned int inputType, InputInfo *info)
{

	if (inputType & EnumeratedAction)
		return PASS_EVENT_THROUGH;

    if (inputType & KeyStroke)
    {   switch (info->val.i)
		{	case 's': // s - toggle stereo (if stereo enabled)
				stereo_flag = -stereo_flag;
				Display();
				return SINK_EVENT;
		}
		return SINK_EVENT;
	}

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/**	Return the window's width
 */
int ToglWindow::getWidth()
{
	return width;
}

/*---------------------------------------------------------------------------*/
/**	Return the window's height
 */
int ToglWindow::getHeight()
{
	return height;
}

/*---------------------------------------------------------------------------*/
/**	Return the window's minimum dimension
 */
int ToglWindow::getMinDimension()
{
	if (height < width)
		return height;
	else
		return width;
}

/*---------------------------------------------------------------------------*/
/**
 *	Sets the current Togl Window for subsequent GL commands.  DOES NOT SET
	THE ToglWindow's myTogl VARIABLE!
 */
void ToglWindow::SetTogl(Togl *togl)
{
//	LogState ls(&gzlog,"ToglWindow::SetTogl",TRACE);
//	LogState ls(&gzlog,"ToglWindow::SetTogl",BOMBASTIC);

	ToglWindow *myself = (ToglWindow *)Togl_GetClientData(togl);

	toglIsValid = Togl_MakeCurrent(togl);
	if (myself)
	{
            if(toglIsValid)
            {
                if(!myself->context)
                    myself->context = gz4d::glo::Context::create();
                //myself->context->activate();
            }
            glViewport(myself->xOffset, myself->yOffset, myself->width, myself->height);
//          gzlog << "size: " << myself->width << ", " << myself->height << endl;
	}
}

/*--------------------------Tcl Utility Procedures---------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	This procedure makes it easier to get error messages when evaluating Tcl
 *	scripts.
 */
int ToglWindow::Tcl_Eval(std::string const &command, std::string &retval)
{
	if (::Tcl_Eval(interp, command.c_str()) != TCL_OK)
	{
		retval = Tcl_GetStringResult(interp);

		::Tcl_Eval(interp, "bgerror");
		return TCL_ERROR;
	}
	retval = Tcl_GetStringResult(interp);
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure makes it easier to get error messages when linking variables
 *	to Tcl variables.
 */
int ToglWindow::Tcl_LinkVar(char const *variable, void *address, int type, GLboolean addToGlobal)
{
	Tcl_Interp *interp = Togl_Interp(myTogl);
	char command[MAXCOMMAND + 1];

	if (::Tcl_LinkVar(interp, variable, (char *)address, type) != TCL_OK)
	{
		::Tcl_Eval(interp, "puts \"$errorInfo\"");
		return TCL_ERROR;
	}
	else if (addToGlobal)
	{
		sprintf(command, "lappend globalVariables %s", variable);
		::Tcl_Eval(interp, command);
	}
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure makes it easier to get error messages when linking variables
 *	to Tcl variables.
 */
int ToglWindow::Tcl_LinkVar_Root(char *variable, void *address, int type)
{
	char command[MAXCOMMAND + 1];

	if (::Tcl_LinkVar(interp, variable, (char *)address, type) != TCL_OK)
	{
		::Tcl_Eval(interp, "puts \"$errorInfo\"");
		return TCL_ERROR;
	}
	else
	{
		sprintf(command, "lappend globalVariables %s", variable);
		::Tcl_Eval(interp, command);
	}
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure makes it easier to unlink variables from tcl variables.
 */
int ToglWindow::Tcl_UnlinkVar(char const *variable)
{
	Tcl_Interp *interp = Togl_Interp(myTogl);
	char command[MAXCOMMAND + 1];

	::Tcl_UnlinkVar(interp, (char *) variable);
	sprintf(command, "set idx [lsearch $globalVariables %s];", variable);
	::Tcl_Eval(interp, command);
	::Tcl_Eval(interp, "set globalVariables [lreplace $globalVariables $idx $idx]");

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure makes it easier to unlink variables from tcl variables.
 */
int ToglWindow::Tcl_UnlinkVar_Root(char *variable)
{
	char command[MAXCOMMAND + 1];

	::Tcl_UnlinkVar(interp, (char *) variable);
	sprintf(command, "set idx [lsearch $globalVariables %s];", variable);
	::Tcl_Eval(interp, command);
	::Tcl_Eval(interp, "set globalVariables [lreplace $globalVariables $idx $idx]");

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure makes it easier to update linked tcl variables.
 */
int ToglWindow::Tcl_UpdateLinkedVar(char const *variable)
{
	Tcl_Interp *interp = Togl_Interp(myTogl);

	::Tcl_UpdateLinkedVar(interp, (char *) variable);

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure makes it easier to update linked tcl variables.
 */
int ToglWindow::Tcl_UpdateLinkedVar_Root(char *variable)
{
	::Tcl_UpdateLinkedVar(interp, (char *) variable);
	return TCL_OK;
}

/*---------------------------------Mouse Move--------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This procedure generates a mouse motion event to move the mouse to the
 *	proper desired location.
 */
int ToglWindow::MoveMouse(int x, int y)
{
	char command[MAXCOMMAND];

	sprintf(command, "event generate $toglPath(%s) <Motion> -warp 1 -x %d -y %d",
			Togl_Ident(myTogl), x, y);
	return Tcl_Eval(command);
}

/*---------------------------------------------------------------------------*/
GLboolean ToglWindow::toggleMouseTracking()
{
	sdvcursor->trackCursor ^= GL_TRUE;
	return sdvcursor->trackCursor;
}

/*---------------------------------------------------------------------------*/
GLboolean ToglWindow::getMouseTracking()
{
	return sdvcursor->trackCursor;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	This procedure returns the ToglWindow that corresponds to the passed name.
 */
ToglWindow *ToglWindow::GetToglWindow(Tcl_Interp *interp, const char *name)
{
	char command[MAXCOMMAND];

	pointerForRetrieval = NULL;
	sprintf(command, "$toglPath(%s) setPointerForRetrieval", name);
	::Tcl_Eval(interp, command);

	return pointerForRetrieval;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure sets the pointerForRetrieval to this window.
 */
int ToglWindow::PointerForRetrieval(int argc, char *argv[])
{
	pointerForRetrieval = this;
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This procedure puts the caller into a more-or-less 2D drawing mode,
 *	setting the raster position at the screen location over the passed 3D
 *	coordinates.  These coordinates are passed back through the incoming
 *	variables, and can be used for subsequent glRasterPos3f calls.  Anything
 *	between a beginRaster2D and endRaster2D call is in an identity projection.
 */
GLboolean ToglWindow::beginRaster2D(float &x, float &y, float &z)
{
	GLfloat pos[4];
	GLint vport[4];
	GLboolean valid;

	glRasterPos3f(x, y, z);
	glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &valid);
	if (!valid)
		return GL_FALSE;

	glGetFloatv( GL_CURRENT_RASTER_POSITION, pos);
	glGetIntegerv(GL_VIEWPORT, vport);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	x = (2 * pos[X]/vport[2]) - 1.0;
	y = (2 * pos[Y]/vport[3]) - 1.0;
	z = -1.0;
	glRasterPos3f(x, y, z);

	return valid;
}

/*---------------------------------------------------------------------------*/
/**	This procedure puts the caller into a true 2D drawing mode,
 *	setting the raster position at the passed screen location in 2D
 *	coordinates.  Anything between a beginRaster2D and endRaster2D
 *	call is in an identity projection.
 */
GLboolean ToglWindow::beginRaster2D(float rx, float ry)
{
	GLboolean valid;

	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glRasterPos3f(rx, ry, -1.0);
	glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &valid);
	if (!valid)
		return GL_FALSE;

	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void ToglWindow::endRaster2D()
{
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure prints the passed string at the passed position.  If
 *	'shadow' is true, the text is drawn with a black background.  If
 *	atScreenDepth is true, then the text is drawn at the front of the screen
 *	instead of at the 3D location passed.
 */
static GLubyte blackBitmap[16384] = {0x00};
bool ToglWindow::printAt(float x, float y, float z, std::string text,
						 GLboolean shadow, void *font, int xOffset, int yOffset,
						 GLboolean atScreenDepth)
{
	int offsetx, offsety, length, width, height;
	GLboolean valid;

	if (text.empty())
		return false;

	length = text.size();

	//draw the main portion
	if (atScreenDepth)
		valid = beginRaster2D(x, y, z);
	else
	{
		glRasterPos3f(x, y, z);
		glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &valid);
	}

	if (!valid)
		return false;

	if (shadow)
		glBitmap(0, 0, 0, 0, xOffset + 1, yOffset + 1, NULL);
	else if (xOffset || yOffset)
		glBitmap(0, 0, 0, 0, xOffset, yOffset, NULL);

        GLboolean depthTest;
        glGetBooleanv(GL_DEPTH_TEST,&depthTest);
        bool shadowFirst = !depthTest;

        if(!shadowFirst)
            for (std::string::size_type i = 0; i < text.size(); i++)
                glutBitmapCharacter(font, text[i]);


	//draw the shadow, if desired.
	if (shadow == 2) //draw a box
	{
		if (blackBitmap[0] == 0x00)
			memset(blackBitmap, 0xFF, 16384);
		glPushAttrib(GL_CURRENT_BIT);
		glColor4f(0.0, 0.0, 0.0, 0.5f);
		width = printingWidth(text, font) + 1;
		height = 17;
		
		if(font == GLUT_BITMAP_9_BY_15)
			height = 19;
		if(font == GLUT_BITMAP_8_BY_13)
			height = 17;
		if(font == GLUT_BITMAP_TIMES_ROMAN_10 || font == GLUT_BITMAP_HELVETICA_10)
			height = 14;
		if(font == GLUT_BITMAP_TIMES_ROMAN_24)
			height = 28;
		if(font == GLUT_BITMAP_HELVETICA_12)
			height = 16;
		if(font == GLUT_BITMAP_HELVETICA_18)
			height = 22;
			
		glRasterPos3d(x, y, z);
		glBitmap(width, height, -xOffset, -yOffset, 0, 0, blackBitmap);
		glPopAttrib();
	}
	else if (shadow)
	{

		glPushAttrib(GL_CURRENT_BIT);
		glColor3f(0.0, 0.0, 0.0);
		for (offsetx = 0; offsetx <= 2; offsetx += 2)
		{
			for (offsety = 0; offsety <= 2; offsety += 2)
			{
			    glRasterPos3d(x, y, z);
				if (offsetx != 0 || offsety != 0 || xOffset != 0 || yOffset != 0)
					glBitmap(0, 0, 0, 0, xOffset + offsetx, yOffset + offsety, NULL);
                                for (std::string::size_type i = 0; i < text.size(); i++)
					glutBitmapCharacter(font, text[i]);
			}
		}
		glPopAttrib();
	}

        if(shadowFirst)
            for (std::string::size_type i = 0; i < text.size(); i++)
                glutBitmapCharacter(font, text[i]);

	if (atScreenDepth)
		endRaster2D();

	return true;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure prints the passed string at the passed screen position.  If
 *	'shadow' is true, the text is drawn with a black shadow, and if 'shadow'
 *	is 2, then a black rectangle is drawn behind.  The text is
 *	drawn at the front of the screen.
 */
bool ToglWindow::printAt(float rx, float ry, std::string text, GLboolean shadow, void *font)
{
	int offsetx, offsety, length, width, height, xOffset = 0, yOffset = 0;
	GLboolean valid;

	if (text.empty())
		return false;

	length = text.size();

	//draw the main portion
	valid = beginRaster2D(rx, ry);

	if (!valid)
		return false;

	if (shadow)
		glBitmap(0, 0, 0, 0, xOffset + 1, yOffset + 1, NULL);
	else if (xOffset || yOffset)
		glBitmap(0, 0, 0, 0, xOffset, yOffset, NULL);

        std::string::size_type i;
	for (i = 0; i < text.size(); i++)
	    glutBitmapCharacter(font, text[i]);

	//draw the shadow, if desired.
	if (shadow == 2) //draw a box
	{
		if (blackBitmap[0] == 0x00)
			memset(blackBitmap, 0xFF, 16384);
		glPushAttrib(GL_CURRENT_BIT);
		glColor4f(0.0, 0.0, 0.0, 0.5f);
		width = printingWidth(text, font) + 1;
		height = 17;
		
		if(font == GLUT_BITMAP_9_BY_15)
			height = 19;
		if(font == GLUT_BITMAP_8_BY_13)
			height = 17;
		if(font == GLUT_BITMAP_TIMES_ROMAN_10 || font == GLUT_BITMAP_HELVETICA_10)
			height = 14;
		if(font == GLUT_BITMAP_TIMES_ROMAN_24)
			height = 28;
		if(font == GLUT_BITMAP_HELVETICA_12)
			height = 16;
		if(font == GLUT_BITMAP_HELVETICA_18)
			height = 22;
			
		glRasterPos3f(rx, ry, -1.0);
		glBitmap(width, height, -xOffset, -yOffset, 0, 0, blackBitmap);
		glPopAttrib();
	}
	else if (shadow)
	{

		glPushAttrib(GL_CURRENT_BIT);
		glColor3f(0.0, 0.0, 0.0);
		for (offsetx = 0; offsetx <= 2; offsetx += 2)
		{
			for (offsety = 0; offsety <= 2; offsety += 2)
			{
			    glRasterPos3f(rx, ry, -1.0);
				if (offsetx != 0 || offsety != 0 || xOffset != 0 || yOffset != 0)
					glBitmap(0, 0, 0, 0, xOffset + offsetx, yOffset + offsety, NULL);
				for (i = 0; i < text.size(); i++)
					glutBitmapCharacter(font, text[i]);
			}
		}
		glPopAttrib();
	}

	endRaster2D();

	return true;
}

/*---------------------------------------------------------------------------*/
int ToglWindow::printingWidth(std::string text, void *font)
{
	int length = 0;
        for (std::string::size_type i = 0; i<text.size(); i++)
		length += glutBitmapWidth(font, text[i]);

	return length;
}

/*---------------------------------------------------------------------------*/
void ToglWindow::printFtvrAt(float x, float y, float z, std::string text, GLboolean shadow, GLfloat pitch_adj_angle)
{
	const float scaling_factor = 0.15f;

	glPushMatrix();
		glScalef(scaling_factor,scaling_factor,-scaling_factor);

		glDisable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);

		//draw the main portion
		glTranslatef(x/scaling_factor, y/scaling_factor, z/scaling_factor);
		glRotatef(-pitch_adj_angle, 1.0, 0.0, 0.0);

                for (std::string::size_type i = 0; i < text.size(); i++)
			glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);

	glPopMatrix();
}

char * ToglWindow::GetDatadir()
{
	return datadir;
}

char * ToglWindow::GetAppdir()
{
	return appdir;
}

int ToglWindow::isRTVideoDisabled()
{
	return disableRTVideo;
}
