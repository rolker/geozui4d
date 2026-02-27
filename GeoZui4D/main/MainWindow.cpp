/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// MainWindow.cpp
/** 
 *  This class implements a main viewport Window3D.
 *  Derived from ToglWindow and commented by mdp 7/14/2000
 */
///////////////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include "engine/Object.h"
#include <stdlib.h>
#include "engine/PosableStateBase.h"
#include "engine/WindowMediator.h"
#include "engine/Scene3D.h"
#include "engine/ClutEditor.h"
#include "interactors/RulerTool.h"
#include "interactors/ZoomWindowTool.h"
#include "interactors/RecordingTool.h"
#include "interactors/PlaybackTool.h"
#include "engine/SessionRecorder.h"
#include "interactors/ZoomInteractor.h"
#include "SubWindow.h"
#include "interactors/DoubleClickInteractor.h"
#include "interactors/PickInteractor.h"
#include "interactors/SDV_Cursor.h"
#include "widgets/Sv_Widgets.h"
#include "utilities/DottedAddress.h"
#include "interactors/Tool3DPallette.h"
#ifndef WIN32
    #include <unistd.h>
#endif
#include <filesystem>

#include "engine/Notes.h"


#ifndef WIN32
//#include "engine/Wiimote.h"
//extern Wiimote wiimote;
#include "engine/Spaceball.h"
extern Spaceball spaceball;
#endif

extern char const * GeoZuiVersion;

int MainWindow::interfaceType = GeoZui4D;

//#define STARTING_SCRIPT "./visualizer.tcl"
std::string starting_script = "./visualizer.tcl";
//#define COMMON_SCRIPT_PART 
std::string common_script_part = "\n\
        #check to see if the GEOZUI4D env var is set\n\
        global env\n\
        set cfpath \"\"\n\
        if {[array get env GEOZUI4D] != \"\"} {\n\
        set configfilepath \"$env(GEOZUI4D)\"\n\
                #puts \"$configfilepath found in env var\" \n\
                # if so, look to see if the config file is in the directory it sepcifies\n\
                if {[file exists [file join $configfilepath $configfilename]] == 1} {\n\
            source [file join $configfilepath $configfilename]\n\
            puts  \"$configfilename found in $configfilepath \" \n\
                set cfpath $configfilepath\n\
                }\n\
        }\n\
        #next look in .GeoZui4D in the user's home directory\n\
        set configfilepath {~/.GeoZui4D/}\n\
        #puts \"Looking for [file join $configfilepath $configfilename]\" \n\
        if {[file exists [file join $configfilepath $configfilename]] == 1} {\n\
                source [file join $configfilepath $configfilename]\n\
                puts \"$configfilename found in $configfilepath \" \n\
                set cfpath $configfilepath\n\
        }\n\
        #now, look in the working directory\n\
        set configfilepath {}\n\
        if {[file exists [file join $configfilepath $configfilename]] == 1} {\n\
                source [file join $configfilepath $configfilename]\n\
                puts \"$configfilename found in working directory $configfilepath\" \n\
                set cfpath [pwd]\n\
        }\n\
        set configfilepath $cfpath\n\
        #set datadir [file nativename $datadir] \n\
        set appdir [file nativename $appdir] \n\
        set bindir [file normalize [file dirname [info nameofexecutable]]] \n\
        set mainRight [string match right $mainScreen] \n\
        set tcl_rcFileName [file join $appdir \""+starting_script+"\"]\n";

//static const char *GeoZuiScript =
static const std::string GeoZuiScript = 
        "#first, we should set the correct path to load the procedures...\n\
        #puts \"Initializing paths\" \n\
        set useStereo false\n\
        set appdir [file normalize [file join [file dirname [info nameofexecutable]] \"../lib/GeoZui4D\"]]\n\
        set datadir {}\n\
        set masterAddress 255.255.25.255\n\
        set eyxX 0.0\n\
        set eyxY 0.0\n\
        set eyxZ 0.5\n\
        set mainScreen {}\n\
        set configfilename GeoZui4D.conf\n\
        set guiMode zui\n"+common_script_part;
//        COMMON_SCRIPT_PART;

//static const char *GeoNavScript =
static const std::string GeoNavScript = 
       "#first, we should set the correct path to load the procedures...\n\
        #puts \"Initializing paths\" \n\
        set useStereo true\n\
        set appdir {}\n\
        set datadir {}\n\
        set iorFilename {}\n\
        set mainScreen {}\n\
        set configfilename GeoZui4D.conf\n\
        set guiMode nav\n" + common_script_part;
//        COMMON_SCRIPT_PART;

//static const char *GeoWallScript =
static const std::string GeoWallScript = 
        "#first, we should set the correct path to load the procedures...\n\
        #puts \"Initializing paths\" \n\
        set useStereo false\n\
        set appdir [file normalize [file join [file dirname [info nameofexecutable]] \"../lib/GeoZui4D\"]]\n\
        set datadir {}\n\
        set mainScreen {}\n\
        set configfilename GeoZui4D.conf\n\
        set guiMode wall\n" + common_script_part;
//        COMMON_SCRIPT_PART;

//static const char *GeoCoastPilotScript =
static const std::string GeoCoastPilotScript = 
        "#first, we should set the correct path to load the procedures...\n\
        #puts \"Initializing paths\" \n\
        set useStereo true\n\
        set appdir [file normalize [file join [file dirname [info nameofexecutable]] \"../lib/GeoZui4D\"]]\n\
        set datadir {}\n\
        set eyxX 0.0\n\
        set eyxY 0.0\n\
        set eyxZ 0.5\n\
        set mainScreen {}\n\
        set configfilename GeoZui4D.conf\n\
        set ioPort 0; #disable access from outside applications\n\
        set guiMode cp\n" + common_script_part;
//        COMMON_SCRIPT_PART;


/*-----------------------Constructors/Destructors----------------------------*/
/*---------------------------------------------------------------------------*/
MainWindow::MainWindow()
: Window3D("main")
{
    InputMediator *priorityMediator;
    sdvcursor = new SDV_Cursor(&stereo_flag);

    clutEditor = NULL;

    notes = NULL;
    
    //sdvFile = NULL;

    if (!windowMediator)
        windowMediator = new WindowMediator("Window");
    globalInputMediator->PushInteractor(windowMediator);
    globalInputMediator->PushInteractor(&globalToolbox);

    globalInputMediator->PushInteractor(PickInteractor::getInstance());

    priorityMediator = globalInputMediator->getPriorityMediator();
    priorityMediator->PushInteractor(doubleClickInteractor);

    currentPose->setPoseInfoFromTo(ChangePose, ChangeHomePose);
    currentPose->publishName("main");
    WindowNeighborhood->addChild(currentPose->getAddress());
}

/*---------------------------------------------------------------------------*/
MainWindow::~MainWindow()
{
    delete notes;
// #ifdef __APPLE__
//     cerr << "ERROR!  FIX: to avoid a long crash, calling abort\n"; 
//     abort();
// #endif // __APPLE__

//    Tcl_Finalize();
    //Tcl_Eval("exit");
}

/*---------------------------------------------------------------------------*/
/**
 */
void MainWindow::initTkCallBack(Tcl_Interp *interp)
{
    globalInputMediator->PushInteractor(sdvcursor);
    highLevelEventMediator->PushInteractor(sdvcursor);
    sdvcursor->SetOneEyedFlagPtr(&oneEyedFlag);
}

/*---------------------------------------------------------------------------*/
/**
 *  Allows for child-specific initialization, sets up the Tcl interpreter if
 *  it is not already set up.
 */
void MainWindow::Init(int argc, char *argv[])
{
//    Tcl_Interp *interp;   This is now global - rja
    setupScript = GeoZuiScript;
    parseCommandLine(argc, argv);

    Window3D::Init(argc, argv);

    Tcl_LinkVar("GeoZuiVersion", &GeoZuiVersion, TCL_LINK_STRING | TCL_LINK_READ_ONLY);

//    interp = Togl_Interp(myTogl); // call this AFTER Window3D::Init()
    scene3d->InitTexture(interp);
    scene3d->InitSockets();
    std::shared_ptr<Clut> c = std::dynamic_pointer_cast<Clut>(scene3d->GetTexturePtr(0));
    clutEditor = new ClutEditor("ClutEditor", interp, c);
    clutEditor->setScene3D(scene3d);
    clutEditor->SetTargetToglPtr(myTogl);

    zoomInteractor->RegisterWithTcl(this);
    sessionRecorder->RegisterWithTcl(this);
    widgets->RegisterWithTcl(this);
    scene3d->RegisterWithTcl(this);
    //set up the global toolbox.
    globalToolbox.RegisterWithTcl(this);
    globalToolbox.addTool(new RulerTool);
    globalToolbox.addTool(new ZoomWindowTool);
    globalToolbox.addTool(NULL); //separator
    globalToolbox.addTool(new RecordingTool(sessionRecorder));
    globalToolbox.addTool(new PlaybackTool(sessionRecorder));

    colorKey = new ClutEditor("ColorKey", interp, c, GL_TRUE);
    colorKey->setScene3D(scene3d);
    colorKey->SetTargetToglPtr(myTogl);

    createObjectMenus();

#ifndef WIN32
    //.SetPosable(currentPose);
    spaceball.SetPosable(currentPose);
#endif
}


/* ========================================================================= */
/**
 *  This method parses the command line and modifies argc and argv for
 *  before returning.
 *
 *  @param  argc   Pointer to the number of arguments on the command line.
 *  @param  argv   Pointer to the array of strings containing the arguments.
 */
void MainWindow::parseCommandLine(int argc, char* argv[] )
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-nav") == 0)
        {
            setupScript = GeoNavScript;
            interfaceType = GeoNav3D;
        }
        else if (strcmp(argv[i], "-cp") == 0)
        {
            setupScript = GeoCoastPilotScript;
            interfaceType = GeoCoastPilot;
        }
        else if (strcmp(argv[i], "-wall") == 0)
        {
            setupScript = GeoWallScript;
        }
        else
            startupFiles.push_back(argv[i]);
    }
}

/*----------------------------Input Callbacks--------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  Processes events targeted for this MainWindow.
 *
 *  @param  inputType   Type field of the info structure
 *  @param  info        Full event info structure
 *  @return         0=not handled / 1=handled / -1=handled,remove
 */
int MainWindow::HandleInput(unsigned int inputType, InputInfo *info)
{
    if (inputType == 0 && !startupFiles.empty())
    {
        for(std::vector<std::string>::iterator f = startupFiles.begin(); f != startupFiles.end(); ++f)
            scene3d->AddObject(scene3d->LoadObject(*f, interp,NULL,true),interp);
        startupFiles.clear();
    }
    if (inputType & EnumeratedAction)
    {
        if (inputType == scene3d->sceneResetEvent())
            widgets->ViewHome();
    }

    if (inputType == NoInput)
    {
    }

    return Window3D::HandleInput(inputType, info);
}


/*--------------------------Interface to Textures----------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  This function sets the current Scene3D to the one passed.  Good for future
 *  operations.
 */
void MainWindow::setScene3D(Scene3D *s3d)
{
    if (scene3d)
        globalInputMediator->RemoveInteractor(scene3d, (unsigned long)this);

    scene3d = s3d;
    if (scene3d)
        globalInputMediator->PushInteractor(scene3d, (unsigned long)this);
    if (clutEditor)
        clutEditor->setScene3D(scene3d);
}

/*---------------------------------------------------------------------------*/
/**
 *  This function sets the current ClutEditor to the one passed.  Good for
 *  future operations.
 */
void MainWindow::setClutEditor(ClutEditor *cluted)
{
    clutEditor = cluted;
    clutEditor->setScene3D(scene3d);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void MainWindow::createObjectMenus()
{
    ObjectRegistrationEntry **registry;
    int numObjects, i;
    
    numObjects = Object::getObjectRegistrySize();
    registry = Object::getObjectRegistry();
    
    for (i = 0; i < numObjects; i++)
    {
        if (registry[i]->aliasFor.empty()) //don't make menus for aliases.
            Tcl_Eval("createObjectMenu "+registry[i]->className+" "+registry[i]->contextMenu);
    }
}
