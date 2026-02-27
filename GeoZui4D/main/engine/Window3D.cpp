/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// Window3D.cpp
//
//  This is the abstract class that encapsulates a 3-D OpenGL window with Togl.
//
// Created by mdp 7/14/2000
///////////////////////////////////////////////////////////////////////////////
#include "engine/Window3D.h"

#include <gz4d/glo/ReadPixels.h>

#include "engine/PosableStateBase.h"
#include "objects/NavigationPath.h"
#include "engine/Object.h"
#include "engine/SDV_Light.h"
#include "engine/Scene3D.h"
#include <tcl.h>
#include <tk.h>
#include<gz4d/glo.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <typeinfo>
#include "widgets/Sv_Widgets.h"
#include "engine/ViewManager.h"
#include "engine/WindowMediator.h"
#include "interactors/ZoomInteractor.h"
#include "interactors/ViewNudger.h"
#include "SubWindow.h"
#include "interactors/WindowAnchor.h"
#include "interactors/SDV_Cursor.h"
#include "utilities/DottedAddress.h"
#include "utilities/Addressable.h"
#include "SubWindow.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "objects/CursorObject.h"
#include <string.h>
#include "engine/gzCommandInterface.h"
#include "interactors/InputTentacle.h"
#include "engine/Persistence.h"
#include "engine/TimeControl.h"
#include "engine/Notes.h"
#include "interactors/PointHandle.h"
#include "engine/PosableStateWrapper.h"
#include "MainWindow.h"
#include "VideoRecorder.h"
#include "engine/TextureMaps/TextureMap.h"
#include "utilities/Utilities.h"
#include "interactors/PositionDrag.h"
#include <gz4d/Pixel.h>
#include <gz4d/glo/Color.h>
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/geo/Projection.h>
#include <gz4d/glo/Translate.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/image/io.h>

#define MAXCOMMAND 256

#include "tclLinkage.h"
extern PointHandle *pointHandle;
extern MainWindow *mainWindow;

/*---------------------------------------------------------------------------*/
int Window3D::windowToglInitialized = 0;
bool Window3D::bridgeView = true;
const EventType *Window3D::windowChangedInputType = Interactor::newEventType("windowChanged");

Addressable WindowNeighborhoodAnchor;
DottedAddress *WindowNeighborhood = new DottedAddress(&WindowNeighborhoodAnchor, "Window");
CursorObject *Window3D::linkageCursor = NULL;

float Window3D::currentPriorityTotal = 0;
int Window3D::priorityParticipants = 0;
int Window3D::usePriorities = 0;
ToglWindow *Window3D::otherMovieWindow = NULL;
bool Window3D::otherMovieWindowIsAbove = false;
std::shared_ptr<TextureMap> Window3D::watermark;
std::shared_ptr<TextureMap> Window3D::compass;
std::shared_ptr<TextureMap> Window3D::compassPoints;
bool Window3D::exitFlag = false;
float Window3D::innerCompassX[16], Window3D::innerCompassY[16];
float Window3D::outerCompassX[16], Window3D::outerCompassY[16];
float Window3D::compassAlpha = 0.3;

static gz4d::ogr::SpatialReference wgs84;

/*-----------------------Constructors/Destructors----------------------------*/
/*---------------------------------------------------------------------------*/
/**
*   Constructor.  Initializes the object bookkeeping and input mediation.
*/
Window3D::Window3D(std::string name, Window3D *myParent)
:   ToglWindow(name),scene3d(NULL),vrec(new VideoRecorder()),surfaceColor(0.0,0.0,0.0,0.0)
{
    //initialize the Scene3D pointer
    parent = myParent;

    //initialize the ViewManager
    currentView = new ViewManager(&width, &height, this,
                        &fishtankVR_flag, &headtracking_flag, &stereo_flag, &ssstereo_flag);

    currentView->setMeasurments(&screenWidth_pix, &screenHeight_pix, &screenLeft, &screenRight, &screenTop, &screenBottom, &mainRight);
    currentView->setEyeX(&eyeX);
    currentView->setEyeY(&eyeY);
    currentView->setEyeZ(&eyeZ);
    
    currentPose = currentView->getPosable();

    clippingPlane[0] = clippingPlane[1] = NULL;
    showAsWireframe = false;

    //Subwindow stuff
    subWindows = subWindowTail = NULL;
    visibleSubwindowCount = subWindowCount = 0;
    updateRequired = GL_FALSE;
    overviewWindow = NULL;
    doneDrawingHighRes = GL_FALSE;
    debugWindow = this; //usually, we want to see our own stuff, but when debugging, sometimes we want to see what another window is rendering from our own vantage point.

    if (!linkageCursor)
    {
        linkageCursor = new CursorObject();
        linkageCursor->SetLabel("3D_Cursor");
    }

    //initialize mouse motion stuff
    mouseMotionGrabber = new InputTentacle("mouseMotionGrabber");
    mouseMotionGrabber->setOwner(this);
    mouseMotionGrabber->setFilter(MouseMotion, GL_TRUE);

    cursorValid = GL_FALSE;
    trackCursorAtSurface = 0;
    trackCursorAtZ = 0;

    
    currentPriority = defaultPriority = 1.0;
    priorityReserve = 0.0;
    currentPriorityTotal += currentPriority;
    priorityParticipants++;
    
    
            
    showWatermark = false;
    showCompass = false;
    waterX = 1.0;
    waterY = 0.0;
    waterS = 4.0; //one quarter of the smaller dimension of the screen
    compassX = 1.0;
    compassY = 0.0;
    compassS = 4.0; //one quarter of the smaller dimension of the screen
    
    light = NULL;
}

/*---------------------------------------------------------------------------*/
/**
*   Destructor.
*/
Window3D::~Window3D()
{
    Node *currentNode, *nextNode;
    char command[MAXCOMMAND];

    scene3d->UnlockHighRes(this); //release our high-res lock, if we have it.

    /* Delete subwindows (nodes taken care of automatically) */
    currentNode = subWindows;
    while (currentNode)
    {
        nextNode = currentNode->next;
        sprintf(command, "subwindowDestroy %s", Togl_Ident(currentNode->window->GetTogl()));
        Tcl_Eval(command);
        currentNode = nextNode;
    }

    /* remove self from the window mediator's list */
    windowMediator->RemoveWindow(this);

    delete light;

    delete currentView;

    currentPriorityTotal -= currentPriority;
    priorityParticipants--;
}

/*---------------------------------------------------------------------------*/
void Window3D::publishName(std::string name)
{
    DottedAddress *oAddress;

    Interactor::publishName(name);

    oAddress = mouseMotionGrabber->getAddress();
    if (oAddress)
        oAddress->reparent(getAddress());
}

/*---------------------------------------------------------------------------*/
/**
*   Allows for child-specific initialization, sets up the Tcl interpreter if
*   it is not already set up.
*/
void Window3D::Init(int argc, char *argv[])
{
    char const *newCommand[] = {"overview", "restoreSubwindows", "limitScale",
                        "createSubwindow", "lockTo", "inParent", "debugWindow",
                        "setHeightExaggeration", "plotPoint", "clippingPlane",
                        "getHeightExaggeration", "useHeightExaggeration",
                        "distanceToFarPlane", "getEyepoint", "perspective",
                        "wireframe", "localHeightExaggeration", "maxHeadingChange",
                        "priority", "perspectiveWidthRatio", "perspectiveHeightRatio",
                        "screenSpecs", "sreenToEye", "videoRecorder", "showWatermark",
                        "setWatermark", "showCompass", "screenShot", "surfaceColor",
                        NULL};
    int currentCommand, i;
    InputMediator *priorityMediator;
    DottedAddress *address;
    PosableStateLock *parentLock;
    
    ToglWindow::Init(argc, argv);

    notes = mainWindow->notes;
    if (!notes)
        notes = new Notes(this);

    // Initialize Togl commands for objects
    if (!windowToglInitialized)
    {
        for (currentCommand = 0; newCommand[currentCommand]; currentCommand++)
            Togl_CreateCommand(newCommand[currentCommand], SubCommandCallBack);
        windowToglInitialized = 1;

        Tcl_LinkVar("viewMovesWithWindow", &ViewManager::viewMovesWithWindow);

        Tcl_Eval("addOption {Subwindow Perspective Tracking} checkbutton "
                "{} viewMovesWithWindow");
        Tcl_Eval("addOption {}");
    }

    // init widgets
    light = new SDV_Light(currentPose);
    address = currentPose->getAddress();
    address->addChild(currentView->getPerspectivePosable()->getAddress());
    address->addChild(currentView->getLockToObject()->getAddress());
    parentLock = currentView->getLockToParent();
    if (parentLock)
        address->addChild(parentLock->getAddress());

    widgets = new Sv_Widgets(currentView, (float)(17.0*CMTOPIXEL), MouseButton1, MouseButton3);

    widgets->SetVelocityRatio(SV_WIDGETS_YROTATION_WIDGET, 1.5);

    //Initialize the zoomInteractor and view nudger
    zoomInteractor = new ZoomInteractor(currentView);
    viewNudger = new ViewNudger(currentView);

    if(widgets == NULL || zoomInteractor == NULL || viewNudger == NULL)
    {
        std::cerr << "Out of memory for creating interactors!\n";
        exit(1);
    }

    if (MainWindow::getInterfaceType() == GeoCoastPilot)
    {
        positionDragger = new PositionDrag();
        inputMediator.PushInteractor(positionDragger);
    }
    else
        positionDragger = NULL;

    //Set up the interactors on the various input stacks.
    inputMediator.PushInteractor(viewNudger);
    inputMediator.PushInteractor(zoomInteractor);
    priorityMediator = inputMediator.getPriorityMediator();
    priorityMediator->PushInteractor(widgets);
    priorityMediator->PushInteractor(currentView);
    priorityMediator->PushInteractor(mouseMotionGrabber);
    windowMediator->PushWindow(this);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0,0.0,0.0,0.0);
        
    glMatrixMode(GL_PROJECTION);
    glEnable(GL_NORMALIZE);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);

    light->Init();      

    address = getAddress();
    widgets->getAddress()->reparent(address);
    zoomInteractor->getAddress()->reparent(address);
    viewNudger->getAddress()->reparent(address);

    currentView->applyFrustum();

    if (!watermark)
    {
        fs::path fname(appdir);
        fname/="watermark.tif";
        watermark = CreateTexture(fname.string());
        if (watermark)
        {
            watermark->SetFilename(fname.string());
            if (watermark->Load())
            {
                watermark.reset();
            }
        }
    }
    if (!compass)
    {
        fs::path fname(appdir);
        fname/="Compass_rose_pale.tif";
        compass = CreateTexture(fname.string());
        if (compass)
        {
            compass->SetFilename(fname.string());
            if (compass->Load())
            {
                compass.reset();
            }
        }
        for (i = 0; i < 16; i++)
        {
            innerCompassX[i] = 0.5 + 3 * sin(i*M_PI/8)/14;
            innerCompassY[i] = 0.5 + 3 * cos(i*M_PI/8)/14;
            outerCompassX[i] = 0.5 + 5 * sin(i*M_PI/8)/14;
            outerCompassY[i] = 0.5 + 5 * cos(i*M_PI/8)/14;
        }
    }
    if (!compassPoints)
    {
        fs::path fname(appdir);
        fname/="CompassPoints.tif";
        compassPoints = CreateTexture(fname.string());
        if (compassPoints)
        {
            compassPoints->SetFilename(fname.string());
            if (compassPoints->Load())
            {
                compassPoints.reset();
            }
        }
    }
}

Window3D *Window3D::cast(ToglWindow *tw)
{
    Window3D *window;

    DYNAMIC_CAST(window, Window3D*, tw);

    return window;
}

/*---------------------------------------------------------------------------*/
void Window3D::updatePosableState()
{
    currentPose = currentView->getPosable();
    widgets->UpdatePose();
}

/*---------------------------------------------------------------------------*/
void Window3D::SetScale(GLfloat sval)
{
    zoomInteractor->SetScale(sval);
}

/*----------------------------Input Callbacks--------------------------------*/
/*---------------------------------------------------------------------------*/
/**
*   This function is the branch-point for Togl subcommands defined by
*   ToglWindow subclasses.  All subclasses must call the parent class's
*   SubCommand procedure after they have exhausted the subcommand
*   possibilities in their implementation of this procedure.
*
*   @param argc Count of argv elements
*   @param argv
*   argv[0]     Togl object name
*   argv[1]     Togl subcommand name (known to be registered)
*   argv[2...]  Arguments
*   @return             TCL_OK / TCL_ERROR
*/
int Window3D::SubCommand(int argc, char *argv[])
{
    float val;
    int plane;
    GLboolean badAction = GL_FALSE;
    GLboolean badSyntax = GL_FALSE;

    if (!strcmp(argv[1], "restoreSubwindows"))
        return RestoreSubwindows();
    else if (!strcmp(argv[1], "overview"))
        return ShowOverviewWindow();
    else if (!strcmp(argv[1], "videoRecorder"))
        return VideoRecorderCommand(argc-2, (const char **)(&argv[2]));
    else if (!strcmp(argv[1], "screenShot"))
    {
        if(argc != 5)
        {
            Tcl_AppendResult(interp, "usage: <window> screenShot width height filename", NULL);
            return TCL_ERROR;
        }
        int w = atoi(argv[2]);
        int h = atoi(argv[3]);
        fs::path fname = argv[4];
        ScreenShot(w,h,fname);
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "surfaceColor"))
    {
        if(argc == 6)
        {
            surfaceColor[0] = atof(argv[2]);
            surfaceColor[1] = atof(argv[3]);
            surfaceColor[2] = atof(argv[4]);
            surfaceColor[3] = atof(argv[5]);
            return TCL_OK;
        }
        Tcl_AppendResult(interp, "usage: <window> surfaceColor r g b a", NULL);
        return TCL_ERROR;
    }
    else if (!strcmp(argv[1], "setWatermark"))
    {
        if(argc == 3)
        {
            std::shared_ptr<TextureMap> newWatermark = CreateTexture(argv[2]);
            if (newWatermark)
            {
                newWatermark->SetFilename(argv[2]);
                if (!newWatermark->Load())
                {
                    watermark = newWatermark;
                    return TCL_OK;
                }
            }
        }
        Tcl_AppendResult(interp, "error setting watermark", NULL);
        return TCL_ERROR;
    }

    else if (!strcmp(argv[1], "showWatermark"))
    {
        if(argc == 2)
            showWatermark = true;
        else
        {
            char value[20] = "";
            int arg = 2;
            if (!strcmp(argv[arg], "X"))
            {
                if (argc > ++arg)
                {
                    waterX = atof(argv[arg++]);
                    showWatermark = true;
                }
                else
                {
                    sprintf(value, "%f", waterX);
                    Tcl_SetResult(interp, value, TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            if (argc > arg && !strcmp(argv[arg], "Y"))
            {
                if (argc > ++arg)
                {
                    waterY = atof(argv[arg++]);
                    showWatermark = true;
                }
                else
                {
                    sprintf(value, "%f", waterY);
                    Tcl_SetResult(interp, value, TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            if (argc > arg && !strcmp(argv[arg], "S"))
            {
                if (argc > ++arg)
                {
                    waterS = atof(argv[arg++]);
                    showWatermark = true;
                }
                else
                {
                    sprintf(value, "%f", waterS);
                    Tcl_SetResult(interp, value, TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            if (argc > arg)
                showWatermark = isTrue( argv[arg]);
        }
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "showCompass"))
    {
        if(argc == 2)
            showCompass = true;
        else
        {
            char value[20] = "";
            int arg = 2;
            if (!strcmp(argv[arg], "X"))
            {
                if (argc > ++arg)
                {
                    compassX = atof(argv[arg++]);
                    showCompass = true;
                }
                else
                {
                    sprintf(value, "%f", compassX);
                    Tcl_SetResult(interp, value, TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            if (argc > arg && !strcmp(argv[arg], "Y"))
            {
                if (argc > ++arg)
                {
                    compassY = atof(argv[arg++]);
                    showCompass = true;
                }
                else
                {
                    sprintf(value, "%f", compassY);
                    Tcl_SetResult(interp, value, TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            if (argc > arg && !strcmp(argv[arg], "S"))
            {
                if (argc > ++arg)
                {
                    compassS = atof(argv[arg++]);
                    showCompass = true;
                }
                else
                {
                    sprintf(value, "%f", compassS);
                    Tcl_SetResult(interp, value, TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            if (argc > arg)
                showCompass = isTrue( argv[arg]);
        }
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "limitScale"))
    {
        val = atof(argv[2]);
        if (val != 0.0)
            currentPose->setScale(val, ChangeMinScale);
        else
            currentPose->unsetPoseInfo(ChangeMinScale);
        val = atof(argv[3]);
        if (val != 0.0)
            currentPose->setScale(val, ChangeMaxScale);
        else
            currentPose->unsetPoseInfo(ChangeMaxScale);
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "setHeightExaggeration"))
    {
        val = atof(argv[2]);
        if (val != 0.0)
            currentView->setHeightExaggeration(val);
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "getHeightExaggeration"))
    {
        char heightEx[20] = "";

        sprintf(heightEx, "%f", currentView->getHeightExaggeration());
        Tcl_SetResult(interp, (char *) heightEx, TCL_VOLATILE);
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "useHeightExaggeration"))
    {
        val = atof(argv[2]);
        if (val != 0.0)
        {
            currentView->setHeightExaggeration(val);
            scene3d->setHeightExaggeration(val);
        }
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "localHeightExaggeration"))
    {
        if (argc >= 3)
        {
            val = atof(argv[2]);
            currentView->setLocalHeightExaggeration(val);
        }
        char cval[20] = "";
        sprintf(cval, "%f", currentView->getHeightExaggeration());
            Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "maxHeadingChange"))
    {
        if (argc >= 3)
        {
            val = atof(argv[2]);
            currentView->resetMaxOrientationChange(val);
        }
        char cval[20] = "";
        sprintf(cval, "%f", currentView->getMaxOrientationChange());
            Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "createSubwindow"))
    {
        SubWindow *subwin;
        gz4d::Point<double> worldPos = currentPose->getPosition();
        if (argc >= 3)
            subwin = new SubWindow(this, worldPos, currentPose->getScale() * 4, argv[2]);
        else
            subwin = new SubWindow(this, worldPos, currentPose->getScale() * 4);
        AddSubwindow(subwin);
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "lockTo"))
    {
        return LockToCommand(argc, argv);
    }
    else if (!strcmp(argv[1], "distanceToFarPlane"))
    {
        if (argc >= 3)
            currentView->setDistanceToFarPlane(atof(argv[2]));
        else
        {
            char distance[20] = "";
            sprintf(distance, "%f", currentView->getDistanceToFarPlane());
            Tcl_SetResult(interp, (char *) distance, TCL_VOLATILE);
        }
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "getEyepoint"))
    {
        char location[60] = "";
        gz4d::Point<double> position = currentView->getEyePoint();

        sprintf(location, "%f %f %f", position[X], position[Y], position[Z]);
        Tcl_SetResult(interp, (char *) location, TCL_VOLATILE);
        return TCL_OK;
    }
    else if (!strcmp(argv[1], "inParent"))
    {
        char retval[8] = "";
        if (argc < 4)
        {
            Tcl_AppendResult(interp, "usage: <window> inParent {proxy|tethers|flagpole|eyepoint|center} {show|pick|strength|linewidth} [<val>]", NULL);
            return TCL_ERROR;
        }
        else
        {
            if (argc == 4)
                val = -2.0f;
            else
                val = atof(argv[4]);
        }
        if (!strcmp(argv[2], "proxy"))
        {
            if (!strcmp(argv[3], "show"))
                sprintf(retval, "%d", currentView->showProxy((int)val));
            else if (!strcmp(argv[3], "pick"))
                sprintf(retval, "%d", currentView->pickProxy((int)val));
            else if (!strcmp(argv[3], "strength"))
                sprintf(retval, "%f", (float)currentView->proxyStrength(val));
            else if (!strcmp(argv[3], "linewidth"))
                sprintf(retval, "%f", (float)currentView->proxyLineWidth(val));
            else
                badAction = GL_TRUE;
        }
        else if (!strcmp(argv[2], "tethers"))
        {
            if (!strcmp(argv[3], "show"))
                sprintf(retval, "%d", currentView->showTethers((int)val));
            else if (!strcmp(argv[3], "strength"))
                sprintf(retval, "%f", currentView->tetherStrength(val));
            else
                badAction = GL_TRUE;
        }
        else if (!strcmp(argv[2], "flagpole"))
        {
            if (!strcmp(argv[3], "show"))
                sprintf(retval, "%d", currentView->showFlagpole((int)val));
            else if (!strcmp(argv[3], "strength"))
                sprintf(retval, "%f", currentView->flagpoleStrength(val));
            else
                badAction = GL_TRUE;
        }
        else if (!strcmp(argv[2], "eyepoint"))
        {
            if (!strcmp(argv[3], "show"))
                sprintf(retval, "%d", currentView->showEyepoint((int)val));
            else
                badAction = GL_TRUE;
        }
        else if (!strcmp(argv[2], "center"))
        {
            if (!strcmp(argv[3], "show"))
                sprintf(retval, "%d", currentView->showCenter((int)val));
            else if (!strcmp(argv[3], "strength"))
                sprintf(retval, "%f", currentView->centerStrength(val));
            else
                badAction = GL_TRUE;
        }
        else
        {
            Tcl_AppendResult(interp, "Invalid argument ", argv[2], ".  Must be proxy, tethers, or flagpole.", NULL);
            return TCL_ERROR;
        }
        if (badAction)
        {
            Tcl_AppendResult(interp, argv[3], " is not a valid action for ", argv[2], ".", NULL);
            if (argv[2][0] == 'p')
                Tcl_AppendResult(interp, "Use show, strength, linewidth, or pick", NULL);
            else
                Tcl_AppendResult(interp, "Use show or strength", NULL);
            return TCL_ERROR;
        }
        else
        {
            Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
            return TCL_OK;
        }
    }
    else if (!strcmp(argv[1], "plotPoint"))
    {
        char retval[64] = "";
        gz4d::Point<double> input, output;
        int errval;

        if (argc < 6)
        {
            Tcl_AppendResult(interp, "usage: <window> plotPoint {world|screen} <x> <y> {<z>|<depth>}", NULL);
            return TCL_ERROR;
        }
        input[X] = atof(argv[3]);
        input[Y] = atof(argv[4]);
        input[Z] = atof(argv[5]);
        if (strcmp(argv[2], "world") == 0)
            errval = currentView->plotWorldPoint(input, output);
        else if (strcmp(argv[2], "screen") == 0)
            errval = currentView->getWorldPoint(input, output);
        else
        {
            Tcl_AppendResult(interp, "Invalid argument ", argv[2], ".  Must be world or screen.", NULL);
            return TCL_ERROR;
        }
        if (errval == GL_TRUE)
            sprintf(retval, "%f %f %f", output[X], output[Y], output[Z]);
        //else, we return an empty string.
        Tcl_SetResult(interp, (char *) retval, TCL_VOLATILE);
        return TCL_OK;
    }
    if (!strcmp(argv[1], "clippingPlane"))
    {
        if (argc < 4)
            badSyntax = GL_TRUE;
        else
        {
            plane = atoi(argv[2]);
            if (!strcmp(argv[3], "OFF"))
            {
                if (clippingPlane[plane])
                {
                    delete clippingPlane[plane];
                    clippingPlane[plane] = NULL;
                }
            }
            else if (argc >= 6)
            {
                if (!clippingPlane[plane])
                    clippingPlane[plane] = new double[4];
                clippingPlane[plane][X] = atof(argv[3]);
                clippingPlane[plane][Y] = atof(argv[4]);
                clippingPlane[plane][Z] = 0.0;
                clippingPlane[plane][3] = atof(argv[5]);
            }
            else
                badSyntax = GL_TRUE;
        }

        if (badSyntax)
        {
            Tcl_AppendResult(interp, "usage: <window> clippingPlane <plane#> [OFF | <x> <y> <distance-from-center>]", NULL);
            return TCL_ERROR;
        }
        return TCL_OK;
    }
    if (!strcmp(argv[1], "perspective"))
    {
        if (argc >= 3)
            currentView->setPerspective(atoi(argv[2]));
        else
        {
            char cval[10] = "";
            sprintf(cval, "%d", currentView->getPerspective());
            Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
        }
        return TCL_OK;
    }
    if (!strcmp(argv[1], "wireframe"))
    {
        if (argc >= 3)
            showAsWireframe = atoi(argv[2]);
        else
        {
            char cval[10] = "";
            sprintf(cval, "%d", showAsWireframe);
            Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
        }
        return TCL_OK;
    }
    if (!strcmp(argv[1], "priority")) //priority [restore | [default | nudge] [<value>]]
    {
        char cval[24] = "";
        if (argc == 2)
            sprintf(cval, "%f", currentPriority);
        else
        {
            if (!strcmp(argv[2], "default"))
            {
                if (argc >= 4)
                    setDefaultPriority((float)atof(argv[3]));
                sprintf(cval, "%f", defaultPriority);
            }
            else
            {
                if (!strcmp(argv[2], "restore"))
                    restoreDefaultPriority();
                else if (!strcmp(argv[2], "nudge"))
                {
                    if (argc >= 4)
                        priorityReserve += (float) atof(argv[3]);
                    else
                        priorityReserve += 1.0;
                }
                else
                    setCurrentPriority((float)atof(argv[2]));
                sprintf(cval, "%f", currentPriority);
            }
        }
        Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
        return TCL_OK;
    }
    if (!strcmp(argv[1], "perspectiveWidthRatio"))
    {
        if (argc >= 3)
            currentView->setPerspectiveWidthRatio(atof(argv[2]));
        char cval[24] = "";
        sprintf(cval, "%f", currentView->getPerspectiveWidthRatio());
        Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
        return TCL_OK;
    }
    if (!strcmp(argv[1], "perspectiveHeightRatio"))
    {
        if (argc >= 3)
            currentView->setPerspectiveHeightRatio(atof(argv[2]));
        char cval[24] = "";
        sprintf(cval, "%f", currentView->getPerspectiveHeightRatio());
        Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
        return TCL_OK;
    }
    if (!strcmp(argv[1], "screenToEye"))
    {
        if (argc >= 5)
        {
            double *xyz = new double[3];
            xyz[0] = atof(argv[2]);
            xyz[1] = atof(argv[3]);
            xyz[2] = atof(argv[4]);
            currentView->setEyeX(&xyz[0]);
            currentView->setEyeY(&xyz[1]);
            currentView->setEyeZ(&xyz[2]);
        }
        else if (argc <= 2)
        {
            char cval[128] = "";
            sprintf(cval, "%f %f %f",
               currentView->getEyeX(),
               currentView->getEyeY(),
               currentView->getEyeZ());
            Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
            return TCL_OK;
        }
        else
        {
            Tcl_AppendResult(interp, "usage: <window> screenToEye <x> <y> <z>", NULL);
            return TCL_ERROR;
        }
        return TCL_OK;
    } 
    if (!strcmp(argv[1], "screenSpecs"))
    {
        if (argc >= 8)
        {
            int *xy = new int[2];
            double *lrtb = new double[4];
            xy[X] = atoi(argv[2]);
            xy[Y] = atoi(argv[3]);
            lrtb[0] = atof(argv[4]);
            lrtb[1] = atof(argv[5]);
            lrtb[2] = atof(argv[6]);
            lrtb[3] = atof(argv[7]);
            currentView->setMeasurments(xy, &xy[Y], lrtb, &lrtb[1], &lrtb[2], &lrtb[3], &mainRight);
        }
        else if (argc <= 2)
        {
            char cval[128] = "";
            sprintf(cval, "%i %i %f %f %f %f", 
               currentView->getScreenWidth(),
               currentView->getScreenHeight(),
               currentView->getScreenLeft(),
               currentView->getScreenRight(),
               currentView->getScreenTop(),
               currentView->getScreenBottom());
            Tcl_SetResult(interp, (char *) cval, TCL_VOLATILE);
            return TCL_OK;
        }
        else
        {
            Tcl_AppendResult(interp, "usage: <window> screenSpecs <width> <height> <left> <right> <top> <bottom>", NULL);
            return TCL_ERROR;
        }
        return TCL_OK;
    }
    if (!strcmp(argv[1], "debugWindow"))
    {
        Addressable *address = NULL;
        Window3D *win = NULL;
        int retval;
        if (argc != 3)
        {
            Tcl_AppendResult(interp, "usage: <window> debugWindow <window>", NULL);
            return TCL_ERROR;
        }
        retval = gzCommandInterface::findWindow(argv[2], address, win);
        if (retval == TCL_OK)
            debugWindow = win;
        return retval;
    }

    return ToglWindow::SubCommand(argc, argv);
}

/*---------------------------------------------------------------------------*/
/**
*   This function creates or modifies a lock between this window and the
*   Posable named by the passed address.
*
*   @param argc Count of argv elements
*   @param argv
*   argv[2]     Dotted address of target posable to be altered
*   argv[3]...  lock-type string (white space allowed):
*                        (t)T - (Delta-)Lock translation
*                        (h)H - (Delta-)Lock heading
*                        (p)P - (Delta-)Lock pitch
*                        (r)R - (Delta-)Lock roll
*                        (s)S - (Delta-)Lock scale
*                       For instance: "THPR" or "Thpr" or "T" or "th"
*   @return             TCL_OK / TCL_ERROR
*/
int Window3D::LockToCommand(int argc, char *argv[])
{
    PosableStateBase *targetPose;
    long changeMask = PosableStateLock::LockFromOwner;
    RelationshipQueryResult *locks;
    PosableStateLock *lock;
    Addressable *address;
    std::stringstream nameString;
    Object *ob;
    int retval;

    //Right number of arguments?
    if (argc != 4)
    {
        Tcl_AppendResult(interp, "Incorrect use of lockTo subcommand.  Usage options:\n",
                                "<window> lockTo <name> <lock-type>\n",
                        NULL);
        return TCL_ERROR;
    }

    //Do we have a valid posable address?
    retval = gzCommandInterface::findPosable(argv[2], address, targetPose);
    if (retval != TCL_OK)
        return retval;

    //We have a valid address, parse the lockType string
    changeMask = gzCommandInterface::parseCouplingType(argv[3]);
    if (changeMask < 0)
        return TCL_ERROR;

    //We have all valid arguments.  Ensure the existence of the proper lock
    DYNAMIC_CAST(ob, Object *, targetPose->getUser());
    if ((changeMask & PosableStateLock::LockLocalized) &&
        currentView->getObjectLockedTo() == NULL && ob != NULL)
        currentView->lockTo(ob, changeMask);
    else
    {
        locks = relationshipManager->getLocksBetween(targetPose, currentPose);
        if (locks->numResults() == 0)
        {
            lock = new PosableStateLock(targetPose->getUser(), targetPose,
                                    currentPose, changeMask);
            nameString << "Coupling_"  << gzCommandInterface::getUniqueCount();
            lock->publishName(nameString.str());
            CouplingNeighborhood->addChild(lock->getAddress());
        }
        else //assume first one is good enough for us.
        {
            lock = (PosableStateLock*) locks->first();
            lock->lock(changeMask);
        }
        delete locks;
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function tells the viewpoint that there has been a change and
*   updates the other reshape information as normal.
*/
void Window3D::Reshape(int x, int y, int w, int h)
{
    ToglWindow::Reshape(x,y,w,h);
    currentView->resetViewportOffsets(x, y);
    currentView->viewUpdated();
    widgets->Resize();
}

/*---------------------------------------------------------------------------*/
void Window3D::setDefaultPriority(float p)
{
    if (p == defaultPriority && defaultPriority == currentPriority)
        return;

    setCurrentPriority(p);
    defaultPriority = p;
}

/*---------------------------------------------------------------------------*/
void Window3D::setPriorityInteractive()
{
    if (!usePriorities)
        return;

    setCurrentPriority(priorityParticipants * currentPriorityTotal + 1);
}

/*---------------------------------------------------------------------------*/
void Window3D::setCurrentPriority(float p)
{
    if (p < 0.0)
        p = 0.0;

    if (p == currentPriority)
        return;

    usePriorities = GL_TRUE;

    if (currentPriority == 0.0)
        priorityParticipants++;

    currentPriorityTotal += (p - currentPriority);
    currentPriority = p;

    if (currentPriority == 0)
        priorityParticipants--;

}

void Window3D::DrawSurface()
{
    if(surfaceColor[3] > 0.0)
    {
        gz4d::glo::PushAttrib attrib(GL_LIGHTING_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        gz4d::Point<double> r = currentView->getReferencePoint();
        //double s = currentView->getPosable()->getScale();

        gz4d::glo::Color(surfaceColor);

        float er = 6371000.0;

        for(float a = 0; a < 45.0; a+=1.0)
        {
            float rad1 = sin(gz4d::Radians(a))*er;
            float rad2 = sin(gz4d::Radians(a+1.0))*er;
            float z1 = -r[2] + (cos(gz4d::Radians(a))*er - er);
            float z2 = -r[2] + (cos(gz4d::Radians(a+1.0))*er - er);

            glBegin(GL_QUAD_STRIP);
            for(int i = 0; i <= 360; i+=12)
            {
                float c = cos(gz4d::Radians(float(i)));
                float s = sin(gz4d::Radians(float(i)));
                glVertex3f(rad2*s,rad2*c,z2);
                glVertex3f(rad1*s,rad1*c,z1);
            }
            glEnd();
        }

    }

}

/*---------------------------------------------------------------------------*/
/**
*   Draws the listed objects to the current display.
*/
void Window3D::Display()
{
    int gl_side[3], currentEye;
    float priorityCheck;

    double savedHintVal = Scene3D::interactivityHint;

    SetTogl(myTogl);
    auto activeContextToken = context->activate();
    if (vrec->isRecordedFrame())
    {
        Scene3D::interactivityHint = 0.0;
                currentView->updateViewInfo();
    }
    else
    {
        glDrawBuffer(GL_BACK);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (usePriorities)
    {
        if (currentPriority <= 0.0f)
            return; //don't draw
        priorityReserve += currentPriority;
        priorityCheck = currentPriorityTotal / priorityParticipants;
        if (priorityReserve < priorityCheck)
            return; //don't draw
        priorityReserve -= priorityCheck;
        if (priorityReserve > priorityCheck)
            priorityReserve = priorityCheck;
    }

    scene3d->UnlockHighRes(this); //release our high-res lock if we had one.
    doneDrawingHighRes = 0;
    updateRequired = GL_FALSE;

    //determine order of rendering.
    if (stereo_flag == 1 && !(vrec->isRecordedFrame()))
    {
        if (sdvcursor->GetDominantEye() == LEFT_EYE)
        {
            gl_side[0] = GL_BACK_RIGHT;
            gl_side[1] = GL_BACK_LEFT;
        }
        else
        {
            gl_side[0] = GL_BACK_LEFT;
            gl_side[1] = GL_BACK_RIGHT;
        }
        gl_side[2] = -1;
    }
    else
    {
        if(vrec->isRecordedFrame())
            gl_side[0] = GL_COLOR_ATTACHMENT0_EXT;
        else
            gl_side[0] = GL_BACK;
        gl_side[1] = -1;
    }

    //render
    for (currentEye = 0; gl_side[currentEye] >= 0; currentEye++)
    {
        transparencyLevel = 0;

        if (currentEye == 0)
            PushTransform(gl_side[currentEye]);
        else
            PushTransform(gl_side[currentEye], 1); //Draw clearing depth buffer bit
        light->Draw();

        //draw solid objects first.
        if (showAsWireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        scene3d->DrawAllLowRes(debugWindow);
        if (showAsWireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        if(!(vrec->isRecordedFrame()))
            notes->Draw(this);
            
        if (linkageCursor->isVisible())
        {
            glPushMatrix();
            glLoadIdentity();
            linkageCursor->getPosable()->applyTransforms(getViewInfo());
            linkageCursor->DrawLowRes(0.0, debugWindow);
            glPopMatrix();
        }
        PopTransform();

        if (scene3d->AreWidgetsVisible())
            widgets->DrawWidgets();

        //  Draw semi-tranparent objects "in front" (transparencyLevel >= 1)
        PushTransform(gl_side[currentEye]);
        while (++transparencyLevel <= scene3d->getMaxTransparencyLevel())
        {
            if (showAsWireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            scene3d->DrawAllLowRes(debugWindow);
            if (showAsWireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        DrawSurface();

        PopTransform();

        DrawSubwindows(gl_side[currentEye]);
    }
    if(!vrec->isRecordedFrame())
    {
        glDrawBuffer(GL_BACK);
    }
    _FirstIdleCall = TRUE;
    transparencyLevel = 0; //set up for high-res loop.
    
    if(oneEyedFlag)
        sdvcursor->DrawCursor(debugWindow);

    drawOrtho();
    drawWatermark();
    drawCompass(true);

    if (!vrec->isRecordedFrame())
    {
        Togl_SwapBuffers(myTogl);
        if(oneEyedFlag)
            sdvcursor->DrawCursor(this);
    }
    else
        {
        Scene3D::interactivityHint = savedHintVal;
        }
}

void Window3D::drawOrtho()
{

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);//left,right,bottom,top

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    scene3d->DrawAllOrtho();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

void Window3D::drawWatermark()
{
    if (watermark && showWatermark)
    {
        glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
            //GLint df;
            //glGetIntegerv(GL_DEPTH_FUNC,&df);
            //glDepthFunc(GL_ALWAYS);
    /*        beginRaster2D(1.0 - (2.0*watermark->GetWidth()/float(width)), -1.0);
        watermark->Draw();
        endRaster2D();*/
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        float l = waterX * (1.0 - waterS);
        float r = waterX + (1.0 - waterX) * waterS;
        float b = waterY * (1.0 - waterS);
        float t = (1.0 - waterY) * waterS + waterY;
        if(width > height)
        {
            l = waterX * (1.0 - waterS*width/(float)height);
            r = waterX + (1.0 - waterX) * (waterS*width/(float)height);
        }
        else
        {
            b = waterY * (1.0 - waterS*width/(float)height);
            t = (1.0 - waterY) * waterS * height/(float)width + waterY;
        }
        gluOrtho2D(l,r,b,t);
            //gluOrtho2D(-1,1,-1,1);
/*        glColor3f(1.0,0.0,0.0);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0,.5);
        glVertex2f(-.5,-.5);
        glVertex2f(.5,-.5);
        glEnd();*/
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        watermark->Draw(true);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
            //glDepthFunc(df);
        glPopAttrib();
    }
}

//special compass at the front
void Window3D::drawCompass(bool resetAlpha)
{
    if (compass && showCompass)
    {
        glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
            //GLint df;
            //glGetIntegerv(GL_DEPTH_FUNC,&df);
            //glDepthFunc(GL_ALWAYS);
    /*        beginRaster2D(1.0 - (2.0*watermark->GetWidth()/float(width)), -1.0);
        watermark->Draw();
        endRaster2D();*/
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        float l = compassX * (1.0 - compassS);
        float r = compassX + compassX * compassS;
        float b = compassY * (1.0 - compassS);
        float t = (1.0 - compassY) * compassS + compassY;
        if(width > height)
        {
            l = compassX * (1.0 - compassS*width/(float)height);
            r = compassX + (1.0 - compassX) * (compassS*width/(float)height);
        }
        else
        {
            b = compassY * (1.0 - compassS*width/(float)height);
            t = (1.0 - compassY) * compassS * height/(float)width + compassY;
        }
        gluOrtho2D(l,r,b,t);
        glEnable(GL_ALPHA_TEST);
        /*
        if (resetAlpha)
            compassAlpha = 0.3f;
        else
            compassAlpha = 0.0f;
        */
        compassAlpha = 0.3f;
        glAlphaFunc(GL_GREATER, compassAlpha);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //gluOrtho2D(-1,1,-1,1);
/*        glColor3f(1.0,0.0,0.0);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0,.5);
        glVertex2f(-.5,-.5);
        glVertex2f(.5,-.5);
        glEnd();*/
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.5f, 0.5f, 0.0);
        glRotated(currentPose->getOrientationAxis()[X]*8.0/9.0 - 80.0, 1.0, 0.0, 0.0);
        glRotated(currentPose->getOrientationAxis()[Y], 0.0, 0.0, 1.0);
        glTranslatef(-0.5f, -0.5f, 0.0);
        //compass->Draw(true);
        compass->Enable();
        glColor3f(1.0,1.0,1.0);
        int i;
        //Center Portion
        glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(0.5, 0.5);
            glVertex2f(0.5, 0.5);
            for (i = 0; i < 16; i++)
            {
                glTexCoord2f(innerCompassX[i], innerCompassY[i]);
                glVertex2f(innerCompassX[i], innerCompassY[i]);
            }
            glTexCoord2f(innerCompassX[0], innerCompassY[0]);
            glVertex2f(innerCompassX[0], innerCompassY[0]);
        glEnd();
        //Sloping Ring
        glBegin(GL_QUAD_STRIP);
            for (i = 0; i < 16; i++)
            {
                glTexCoord2f(innerCompassX[i], innerCompassY[i]);
                glVertex3f(innerCompassX[i], innerCompassY[i], 0.0f);
                glTexCoord2f(outerCompassX[i], outerCompassY[i]);
                glVertex3f(outerCompassX[i], outerCompassY[i], 0.1f);
            }
            glTexCoord2f(innerCompassX[0], innerCompassY[0]);
            glVertex3f(innerCompassX[0], innerCompassY[0], 0.0f);
            glTexCoord2f(outerCompassX[0], outerCompassY[0]);
            glVertex3f(outerCompassX[0], outerCompassY[0], 0.1f);
        glEnd();
        //Top-left outer corner
        glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(0.0, 1.0);
            glVertex3f(0.0, 1.0, 0.1f);
            glTexCoord2f(0.0, 0.5);
            glVertex3f(0.0, 0.5, 0.1f);
            for (i = 12; i < 16; i++)
            {
                glTexCoord2f(outerCompassX[i], outerCompassY[i]);
                glVertex3f(outerCompassX[i], outerCompassY[i], 0.1f);
            }
            glTexCoord2f(outerCompassX[0], outerCompassY[0]);
            glVertex3f(outerCompassX[0], outerCompassY[0], 0.1f);
            glTexCoord2f(0.5, 1.0);
            glVertex3f(0.5, 1.0, 0.1f);
        glEnd();
        //Top-right outer corner
        glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(1.0, 1.0);
            glVertex3f(1.0, 1.0, 0.1f);
            glTexCoord2f(0.5, 1.0);
            glVertex3f(0.5, 1.0, 0.1f);
            for (i = 0; i <= 4; i++)
            {
                glTexCoord2f(outerCompassX[i], outerCompassY[i]);
                glVertex3f(outerCompassX[i], outerCompassY[i], 0.1f);
            }
            glTexCoord2f(1.0, 0.5);
            glVertex3f(1.0, 0.5, 0.1f);
        glEnd();
        //Bottom-right outer corner
        glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(1.0, 0.0);
            glVertex3f(1.0, 0.0, 0.1f);
            glTexCoord2f(1.0, 0.5);
            glVertex3f(1.0, 0.5, 0.1f);
            for (i = 4; i <= 8; i++)
            {
                glTexCoord2f(outerCompassX[i], outerCompassY[i]);
                glVertex3f(outerCompassX[i], outerCompassY[i], 0.1f);
            }
            glTexCoord2f(0.5, 0.0);
            glVertex3f(0.5, 0.0, 0.1f);
        glEnd();
        //Bottom-left outer corner
        glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(0.0, 0.0);
            glVertex3f(0.0, 0.0, 0.1f);
            glTexCoord2f(0.5, 0.0);
            glVertex3f(0.5, 0.0, 0.1f);
            for (i = 8; i <= 12; i++)
            {
                glTexCoord2f(outerCompassX[i], outerCompassY[i]);
                glVertex3f(outerCompassX[i], outerCompassY[i], 0.1f);
            }
            glTexCoord2f(0.0, 0.5);
            glVertex3f(0.0, 0.5, 0.1f);
        glEnd();
        compass->Disable();
        //North Point
        glPushMatrix();
        glTranslatef(0.5f, 1.1f, 0.2f);
        glRotated(-currentPose->getOrientationAxis()[Y], 0.0, 0.0, 1.0);
        glRotated(90.0 - currentPose->getOrientationAxis()[X]*8.0/9.0, 1.0, 0.0, 0.0);
        compassPoints->Enable();
        glBegin(GL_QUADS);
            glTexCoord2f(0.0, 1.0);
            glVertex3f(-0.15, 0.1, 0.0f);
            glTexCoord2f(0.6, 1.0);
            glVertex3f(0.15, 0.1, 0.0f);
            glTexCoord2f(0.6, 0.53);
            glVertex3f(0.15, -0.1, 0.0f);
            glTexCoord2f(0.0, 0.53);
            glVertex3f(-0.15, -0.1, 0.0f);
        glEnd();
        compassPoints->Disable();
        glPopMatrix();
        //East Point
        glPushMatrix();
        glTranslatef(1.07f, 0.5f, 0.19f);
        glRotated(-currentPose->getOrientationAxis()[Y], 0.0, 0.0, 1.0);
        glRotated(90.0 - currentPose->getOrientationAxis()[X]*8.0/9.0, 1.0, 0.0, 0.0);
        compassPoints->Enable();
        glBegin(GL_QUADS);
            glTexCoord2f(0.6, 1.0);
            glVertex3f(-0.07, 0.07, 0.0f);
            glTexCoord2f(1.0, 1.0);
            glVertex3f(0.07, 0.07, 0.0f);
            glTexCoord2f(1.0, 0.52);
            glVertex3f(0.07, -0.07, 0.0f);
            glTexCoord2f(0.6, 0.52);
            glVertex3f(-0.07, -0.07, 0.0f);
        glEnd();
        compassPoints->Disable();
        glPopMatrix();
        //West Point
        glPushMatrix();
        glTranslatef(-0.07f, 0.5f, 0.2f);
        glRotated(-currentPose->getOrientationAxis()[Y], 0.0, 0.0, 1.0);
        glRotated(90.0 - currentPose->getOrientationAxis()[X]*8.0/9.0, 1.0, 0.0, 0.0);
        compassPoints->Enable();
        glBegin(GL_QUADS);
            glTexCoord2f(0.0, 0.51);
            glVertex3f(-0.105, 0.068, 0.0f);
            glTexCoord2f(0.6, 0.51);
            glVertex3f(0.105, 0.068, 0.0f);
            glTexCoord2f(0.6, 0.0);
            glVertex3f(0.105, -0.068, 0.0f);
            glTexCoord2f(0.0, 0.0);
            glVertex3f(-0.105, -0.068, 0.0f);
        glEnd();
        compassPoints->Disable();
        glPopMatrix();
        //South Point
        glPushMatrix();
        glTranslatef(0.5f, -0.07f, 0.2f);
        glRotated(-currentPose->getOrientationAxis()[Y], 0.0, 0.0, 1.0);
        glRotated(90.0 - currentPose->getOrientationAxis()[X]*8.0/9.0, 1.0, 0.0, 0.0);
        compassPoints->Enable();
        glBegin(GL_QUADS);
            glTexCoord2f(0.6, 0.50);
            glVertex3f(-0.07, 0.07, 0.0f);
            glTexCoord2f(1.0, 0.50);
            glVertex3f(0.07, 0.07, 0.0f);
            glTexCoord2f(1.0, 0.02);
            glVertex3f(0.07, -0.07, 0.0f);
            glTexCoord2f(0.6, 0.02);
            glVertex3f(-0.07, -0.07, 0.0f);
        glEnd();
        compassPoints->Disable();
        glPopMatrix();

        //cleanup
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glDisable(GL_ALPHA_TEST);
        glAlphaFunc(GL_ALWAYS, 0);
        glBlendFunc(GL_ONE, GL_ZERO);
        glPopMatrix();
            //glDepthFunc(df);
        glPopAttrib();
    }
    /*
    if (MainWindow::getInterfaceType() == GeoCoastPilot)
    {
        glPushMatrix();
        glRotated(orient[Y], 0.0, 0.0, 1.0);
        glPopMatrix();
    }*/
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes idle-time display to occur, for instance zooming
*  or locked-rotation with the widgets, or incremental high-res display.
*/
void Window3D::Idle()
{
    int doneFlag = 1, wasFirstIdleCall = FALSE;
    int gl_side[3], currentEye;
    long eye[2];
    static int callTime = 0;
    
    if (exitFlag) //added to make sure we exit from the right thread (mdp 4/4/2008--doesn't seem to help our crash though)...
    {
        //exit(0);
        Tcl_Exit(0);
    }

    if(notes->isMoving())
    {
        notes->move();
        updateRequired = GL_TRUE;
    }
    
//  if(vrec)
//      updateRequired = GL_TRUE;

    if (doneDrawingHighRes && !updateRequired)
        return;

    if(!currentPose->isInContinuousChange() &&
        !Scene3D::isInContinuousChange() && !updateRequired)
    {
        if (_FirstIdleCall == TRUE)
        {
            _FirstIdleCall = -2; //something to bring us around again
            return;
        }

        if (!scene3d->AssertLockHighRes(this)) //check to see if we can render high res.
            return;

        SetTogl(myTogl);
        auto activeContextToken = context->activate();
        if (_FirstIdleCall)// clear the buffer on the first call
        {
            glDrawBuffer(GL_FRONT_AND_BACK);
            //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

            glClear(GL_DEPTH_BUFFER_BIT);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            _FirstIdleCall = FALSE;
            wasFirstIdleCall = TRUE;
            scene3d->ResetAllHighRes();
        }

        if (oneEyedFlag)
            sdvcursor->RemoveCursor(this);

        //determine order of rendering.
        if (stereo_flag == 1)
        {
            eye[1] = sdvcursor->GetDominantEye();
            eye[0] = 1 - eye[1];
            if (eye[1] == LEFT_EYE)
            {
                gl_side[0] = GL_FRONT_RIGHT;
                gl_side[1] = GL_FRONT_LEFT;
            }
            else
            {
                gl_side[0] = GL_FRONT_LEFT;
                gl_side[1] = GL_FRONT_RIGHT;
            }
            gl_side[2] = -1;
        }
        else
        {
            gl_side[0] = GL_FRONT;
            gl_side[1] = -1;
            eye[0] = MONO_EYE;
        }

        //render
        for (currentEye = 0; gl_side[currentEye] >= 0; currentEye++)
        {
            PushTransform(gl_side[currentEye]);
            light->Draw();
            if (showAsWireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            doneFlag &= scene3d->DrawAllHighRes(eye[currentEye], debugWindow);
            if (showAsWireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if(!vrec->isRecordedFrame())
                notes->Draw(this);

            if ((doneFlag == 1) && linkageCursor->isVisible())
            {
                glPushMatrix();
                glLoadIdentity();
                linkageCursor->getPosable()->applyTransforms(getViewInfo());
                linkageCursor->DrawHighRes(currentEye, debugWindow);
                glPopMatrix();
            }

            if (doneFlag == 1)
                DrawSurface();

            PopTransform();

            if (wasFirstIdleCall && scene3d->AreWidgetsVisible())
                widgets->DrawWidgets();

            if (doneFlag == 1)
                DrawSubwindows(gl_side[currentEye]);
        }
        if (doneFlag == 1)
        {
            //bump us up to the next level of transparency.  If we've reached
            //the max, wrap up.
            if (++transparencyLevel > scene3d->getMaxTransparencyLevel())
            {
                drawOrtho();
                drawWatermark();
                drawCompass();
                doneDrawingHighRes = 1;
                scene3d->UnlockHighRes(this); //release our high-res lock.
            }
            else
                scene3d->ResetAllHighRes();
        }
        if (callTime++ == 40)
        {
            callTime = 0;
            drawCompass();
        }
        if (oneEyedFlag)
            sdvcursor->DrawCursor(this);
        return;
    }
    else
    {
        Display();
    }
}

/*---------------------------------------------------------------------------*/
/**
*   Processes events targeted for this Window3D.
*
*   @param  inputType   Type field of the info structure
*   @param  info        Full event info structure
*   @return             0=not handled / 1=handled / -1=handled,remove
*/
int Window3D::HandleInput(unsigned int inputType, InputInfo *info)
{
    gz4d::Point<double> worldPos;
    Object *ob;

    if (inputType & EnumeratedAction)
    {
        if (inputType == scene3d->addObjectEvent())
            doneDrawingHighRes = 0;
        else if (inputType == scene3d->sceneChangedEvent())
            Display();
        else if (inputType == scene3d->objectChangedEvent())
            updateRequired = GL_TRUE;
        else if (inputType == windowChangedEvent())
            updateRequired = GL_TRUE;
        else if (inputType == currentView->viewChangedEvent())
        {
            updateRequired = GL_TRUE;
            cursorPosAtZValid = GL_FALSE;
            surfaceCursorPosValid = GL_FALSE;
            if ((info->modifier.i & ChangeTranslateMask) && scene3d)
            {
                char positionCommand[256];
                gz4d::Point<double> position = currentPose->getPosition();
                //output lat/lon for workspace center
                double x = position[X], y = position[Y], lat, lon;
                gz4d::ogr::SpatialReference sr = scene3d->getSpatialReference();
                if (sr && !sr.IsGeographic())
                {
                    wgs84.SetWellKnownGeogCS("WGS84");
                    gz4d::geo::Projection proj(wgs84, mainWindow->getScene3D()->getSpatialReference());

                    gz4d::geo::Position<double> ret = proj.inverse(x,y);
                    lat = ret.latitude.value();
                    lon = ret.longitude.value();
                }
                else
                {
                    lon = x;
                    lat = y;
                }
                //lat and lon in decimal degree notation
                DecimalDegrees dd(lat,lon, "\\\"", false);

/*                sprintf(positionCommand, "setWorkspaceLocation %s {%.2f, %.2f, %.2f (%s, %s)}",
                        Togl_Ident(GetTogl()), position[X],
                        position[Y], position[Z], dd.lat_string.c_str(), dd.lon_string.c_str());*/
                sprintf(positionCommand, "setWorkspaceLocation %s {%.2f, %.2f, %.2f (%f, %f)}",
                        Togl_Ident(GetTogl()), position[X],
                                   position[Y], position[Z], lat, lon);
                Tcl_Eval(positionCommand);
                if (MainWindow::getInterfaceType() == GeoCoastPilot)
                {
                    char command[128];
                    sprintf(command, "updateLatLon %d \"%s\" %d \"%s\" %.6f %6f",int(lat),dd.lat_string.c_str(),int(lon),dd.lon_string.c_str(), lat, lon);
                    Tcl_Eval(command);
                }
            }
            if ((info->modifier.i & ChangeRotateMask) && scene3d)
            {
                char rotationCommand[256];
                const OrientationInfo *orient = currentPose->getOrientation();
                float val;

                if (info->modifier.i & ChangeRotateYMask)
                {
                    val = orient->axis[Y];
                    if (val < 0.0)
                        val += 360.0;
                    sprintf(rotationCommand, "setWorkspaceHeading %s {%.2f}",
                            Togl_Ident(GetTogl()), val);
                    Tcl_Eval(rotationCommand);
                }
                if (info->modifier.i & ChangeRotateXMask)
                {
                    sprintf(rotationCommand, "setWorkspacePitch %s {%.2f}",
                            Togl_Ident(GetTogl()), orient->axis[X]);
                    Tcl_Eval(rotationCommand);
                }
            }
            if ((info->modifier.i & ChangeScaleMask) && scene3d)
            {
                currentView->getScreenCenterPoint();
            }
            return SINK_EVENT;
        }
    }
    else
    {
        if (inputType == NoInput && mouseMotionGrabber->getIgnoreBefore() != info->serialNumber)
        {
            Idle();
            mouseMotionGrabber->ignoreBefore(info->serialNumber);
            return PASS_EVENT_THROUGH;
        }

        mouseMotionGrabber->ignoreBefore(info->serialNumber);
        if (inputType & KeyStroke)
        {
        switch (info->val.i)
            {   
                case 'w': // w - make widgets visible/invisible
                    scene3d->SetWidgetsVisible(!scene3d->AreWidgetsVisible());
                    updateRequired = GL_TRUE;
                    globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
                    return SINK_EVENT;
                case 'f': // fuse the objects
                    scene3d->Fuse();
                    globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
                    return SINK_EVENT;
                case 'h': // restore the home position
                    widgets->ViewHome();
                    updateRequired = GL_TRUE;
                    return SINK_EVENT;
                case 'm': // measure a feature
                    getSelectedLocation(info, worldPos);
//                  cout << (currentView->getPixelSizeAt(worldPos)/currentView->getPixelSize()) << endl;
                    std::cout << (currentView->getPixelSizeAt(worldPos)) << std::endl;
                    break;
                case 'c': // toggle the cursor
                    if (toggleMouseTracking())
                        linkageCursor->SetVisible();
                    else
                        linkageCursor->SetInvisible();
                    break;
                case 'd': // toggle whether or not the cursor shows distance
                    linkageCursor->toggleShowDistance();
                    break;
                case 'o': // set orthogonal view
                    currentView->setPerspective(GL_FALSE);
                    break;
                case 'p': // set perspective view
                    currentView->setPerspective();
                    break;
                case 'P': // hack to test path planning
                    NavigationPath *n;
                    n = new NavigationPath();
                    n->SetLabel("navpath");
                    n->insert(Scene3D::timeControl->getGZTime(), 0, 0, 2.0);
                    pointHandle->beginLayout(n);
                    pointHandle->getPointWrapper()->setIndex(1);
                    scene3d->AddObject(n, interp);

                    
                    break;
                case 'B': //back to bridge view
                    if(bridgeView)
                        setBridgeView(false);
                    else
                        setBridgeView();
                    break;
//              case 'n': // notes interface
//                  notes->TogleGUI();
//                  break;
            }
        }
        else if ((inputType & MouseButtonDown) && (inputType & MouseButton3))
        {
            ob = Object::getSelectedObject();
            if (ob)
                ob->postContextMenu(info);
            else
                PostOptionMenu(info->x.i, info->y.i);
            return SINK_EVENT;
        }
        else if (inputType & MouseMotion)
        {
            surfaceCursorPosValid = cursorPosAtZValid = GL_FALSE;
            if ((inputType & MouseLeave) == MouseLeave)
                cursorValid = GL_FALSE;
            else
            {
                //update info
                cursorValid = GL_TRUE;
                cursorX = info->x.i;
                cursorY = info->y.i;

                if (MainWindow::getInterfaceType() == GeoNav3D)
                {
                    //tie cursor values into the cursor Info Bar
                    double x=0.0, y=0.0, lat, lon;

                    //returns x, y in meters
                    getCursorPosOnPlane(x, y, 0);
                    
                    gz4d::ogr::SpatialReference sr = scene3d->getSpatialReference();
                    if (sr && !sr.IsGeographic())
                    {
                        gz4d::ogr::SpatialReference wgs84;
                        wgs84.SetWellKnownGeogCS("WGS84");
                        gz4d::geo::Projection proj(wgs84, mainWindow->getScene3D()->getSpatialReference());
                        gz4d::geo::Position<double> ret = proj.inverse(x,y);
                        lat = ret.latitude.value();
                        lon = ret.longitude.value();
                    }
                    else
                    {
                        lon = x;
                        lat = y;
                    }
                    //lat and lon in decimal degree notation
                    DecimalDegrees dd(lat,lon);
                    char command[128];
                    sprintf(command, "updateLatLon %d %s %d %s %.6f %6f",int(lat),dd.lat_string.c_str(),int(lon),dd.lon_string.c_str(), lat, lon);
                    Tcl_Eval(command);
                    
                    //tie in cursor info for the depth of cursor location
                    char depthStr[128];
                    getSelectedLocation(info,worldPos,NULL,GL_FALSE);
                    int test = (int)(worldPos[Z]);
                    //this is so no long values are printed in dynamic info bar
                    //this should be if(!loaded(gutm))...is there a function for this?
                    
                    if(test == 0)
                        sprintf(depthStr,"000.0m"); 
                    else if (test < -10000000.0)
                        sprintf(depthStr," ");
                    else
                        sprintf(depthStr,"%05.1fm",worldPos[Z]);
                    sprintf(command,"updateDepth %s",depthStr);
                    Tcl_Eval(command);
                    
                    //tie in cursor info to the distance from the boat (center of workspace)
                    //mdp 12/21/2006: Doing this through lat-lon is a bit convoluted, and
                    //drops loads of precision, so I'm changing it.  We already have the
                    //position information we need, we just need to use it!
/*
                    double distance = 0, centerX, centerY, centerLat, centerLon;
                    const double *pos = currentPose->getPosition();

                    centerX = pos[X];
                    centerY = pos[Y];

                    proj = scene3d->getProjection();
                    if(proj)
                    {
                        proj->makeCurrent();
                        proj->invert(centerX,centerY, centerLon, centerLat);
                    }
                    else
                    {
                        centerLon = centerX;
                        centerLat = centerY;
                    }
                    //this isn't working!!! Values are WAY off! And doesn't work when P entered.
                    //calculate distance using decimal degree of two locations
                    distance = proj->calculateDistance(lat,lon,centerLat,centerLon);    

                    if(distance < 0)
                        distance = 0;
*/                
                    double distance = 0, dis;
                    gz4d::Point<double> pos = currentPose->getPosition();
                    int dim;

                    for (dim = X; dim <= Z; dim++)
                    {
                        dis = pos[dim] - worldPos[dim];
                        distance += dis*dis;
                    }
                    distance = sqrt(distance);
                    char distStr[64];
                    if (distance > 1000000000.0) //distances that don't make sense
                        strcpy(distStr,"---.-m");
                    else
                        sprintf(distStr,"%05.1fm",distance);
                    sprintf(command, "updateDistance %s",distStr);
                    Tcl_Eval(command);  

                    // tie in cursor bearing (direction of cursor from boat-center of workspace)
                    double dx, dy, h;
                    dx =  x - pos[X];
                    dy = y - pos[Y];
                    h = atan2(dx,dy);
                    double bearing = h*180/M_PI;
                    if (bearing<0)
                        bearing += 360;
                    if ((int)bearing == 360)
                        bearing = 0;
                    char bearingStr[64];
                    sprintf(bearingStr,"%03.0f",bearing);
                    sprintf(command,"updateBearing %s",bearingStr);
                    Tcl_Eval(command);

                    // tie in cursor heading w/respect to the actual boat direction (not just north)
                    double rBearing = 205.64;
                    char rBearingStr[64];
                    sprintf(rBearingStr,"%03.0f",rBearing);
                    sprintf(command,"updaterBearing %s",rBearingStr);
                    Tcl_Eval(command);

                    // tie in ETR (what is ETR?) time it would take boat to get to cursor location
                    // just straight line estimate, not factoring in wind, currents, turning, etc.
                    double etr = 46865; //in seconds
                    char etrString[64];

                    // this is the format Seconds function from NavigationPath class
                    int hours, minutes, seconds;
                    hours = (int)(etr/3600);
                    minutes = ((int)etr - (hours*3600))/60;
                    seconds = (int)etr - (hours*3600) - (minutes*60);
                    sprintf(etrString,"  %03d:%02d:%02d",hours,minutes,seconds);
                    sprintf(command,"updateETR %s",etrString);

                    //sprintf(command, "updateDynamicInfoBar %s %s %s %s %s %s %s",
                    //  latStr,lonStr,depthStr,distStr,bearingStr,headingStr,etrString);

                    Tcl_Eval(command);
                    
                    //proj->formatDecimalDegrees(lat, valueLat, 1);
                    //proj->formatDecimalDegrees(lon, valueLon, 0);
                    //char command[64];
                    //sprintf(command, "updateInfoBar %s %s",valueLon, valueLat);
                    //Tcl_Eval(command);
                }
            }
        }
    }
    return ToglWindow::HandleInput(inputType, info);
}

/*---------------------------------------------------------------------------*/
void Window3D::setBridgeView(bool on)
{
    if (on == bridgeView)
        return;

    if(on)
    {
        Tcl_Eval("paneForget 1");
        bridgeView = true;
        Tcl_Eval("panePack $bridgeWindow 1");
    }
    else
    {
        Tcl_Eval("paneForget 1");
        bridgeView = false;
        Tcl_Eval("global pathPlanWindow; panePack $pathPlanWindow 1");
    }
}

/*---------------------------------------------------------------------------*/
/**
*   Called whenever a picking round is beginning.  Activates the subwindows'
*   anchors for picking.
*/
void Window3D::startPickingRound(unsigned char pickType)
{
    Node *currentNode;

    if (pickType == (Pickable::PICK_PRESS1))
    {
        WindowAnchor::nextPick();
        for (currentNode = subWindows; currentNode; currentNode = currentNode->next)
            currentNode->window->enablePicking();
    }
    else if (pickType == (Pickable::PICK_CLICK1))
    {
        Object::setSelectedObject(NULL);
        scene3d->resetInfo();
    }
    else if (pickType == (Pickable::PICK_PRESS2))
        Object::setFrameOfReference(NULL);
}

/*---------------------------------------------------------------------------*/
/**
*   Enable's this window's proxy to be picked.
*/
void Window3D::enablePicking()
{
    currentView->enablePicking();
}

/*---------------------------------------------------------------------------*/
/**
*   Using the position info from the passed info structure, determine
*   the 3D location the user was most likely trying to select, and return
*   it in worldPos.  The depth buffer value at the relevant screen position
*   is returned through depth if it is non-NULL.
*
*  @param       info        window, x/y coords, etc.
*   @param      worldPos    Selected 3D position returned through this
*   @param      depth       z-buffer value at selected position is stored here.
*   @param  allowRedisplay  ?
*  @return                 true (worked) / false (failed).
*/
GLboolean Window3D::getSelectedLocation(InputInfo *info, gz4d::Point<double> &worldPos, double *depth, GLboolean allowRedisplay)
{
    gz4d::Point<double> screenPos((double)info->x.i, (double)info->y.i,-1.0);

    if (!currentView->getWorldPoint(screenPos, worldPos))
    {
        if (allowRedisplay)
        {
            Display();
            if (!currentView->getWorldPoint(screenPos, worldPos))
                return GL_FALSE;
        }
        else
            return GL_FALSE;
    }
    if (depth)
        *depth = screenPos[Z];

    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/**
*   This function posts the appropriate option menu for this Window3D.  It
*   makes certain to disable non-sensible options (such as restore subwindows
*   when none need to be restored).
*/
void Window3D::PostOptionMenu(int x, int y)
{
    char command[MAXCOMMAND];

    sprintf(command, "postSubToglMenu %s %d %d {entryconfigure {", Togl_Ident(myTogl), x, y);
    strcat(command, LABEL_RESTORE_SUBS);
    if (subWindows && visibleSubwindowCount != subWindowCount)
        strcat(command, "} -state active}");
    else
        strcat(command, "} -state disabled}");
    Tcl_Eval(command);
}

/*--------------------------Object Bookkeeping-------------------------------*/
/*---------------------------------------------------------------------------*/
/**
*   This function sets the current Scene3D to the one passed.  Good for future
*   operations.
*/
void Window3D::setScene3D(Scene3D *s3d)
{
    scene3d = s3d;
}

/*---------------------------------------------------------------------------*/
/**
*   This function returns the current Scene3D.
*
*   @return             Scene3D Pointer
*/
Scene3D *Window3D::getScene3D()
{
    return scene3d;
}

/*---------------------------------------------------------------------------*/
/**
*   This function adds an object to the current Scene3D, and resets the
*   doneDrawingHighRes flag.
*
*   @param  object      Object to add to the scene.
*   @param  flags       Flags for futuer expansion (only to this window, or to whole scene, for instance).
*/
void Window3D::addObject(Object *object, int flags)
{
    scene3d->AddObject(object, interp);
}

/*--------------------------Viewpoint Operations-----------------------------*/
/*---------------------------------------------------------------------------*/
/**
*   Sets the proper drawing buffer, clears it if requested, and pushes all
*  the appropriate view matrices.
*/
void Window3D::PushTransform(long gl_side, int clearDepth, GLboolean fromScratch)
{
    glDrawBuffer(gl_side);
    glReadBuffer(gl_side);
    if (clearDepth)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    currentView->setEye(gl_side, fromScratch);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    currentView->applyTransforms();
    EnableClippingPlanes();
}

/*---------------------------------------------------------------------------*/
/**
*   Pops the matrices pushed by PushTransform.
*/
void Window3D::PopTransform()
{
    DisableClippingPlanes();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    currentView->viewMatriciesValid = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/**
*   Establishes the clipping planes if there are any
*/
int Window3D::EnableClippingPlanes()
{
    int i, retval = 0;

    //set up any clipping planes
    for (i = 0; i < 2; i++)
    {
        if (clippingPlane[i])
        {
            glPushMatrix();
            gz4d::glo::Translate(currentPose->getPosition());
            glRotatef(-currentPose->getOrientationAxis()[Yaw], 0.0, 0.0, 1.0);
            glClipPlane(GL_CLIP_PLANE0 + i, clippingPlane[i]);
            glEnable(GL_CLIP_PLANE0 + i);
            glPopMatrix();
            retval++;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*/
/**
*   Disables the clipping planes that were set up.
*/
void Window3D::DisableClippingPlanes()
{
    if (clippingPlane[0])
        glDisable(GL_CLIP_PLANE0);
    if (clippingPlane[1])
        glDisable(GL_CLIP_PLANE1);
}

/*--------------------------Cursor Tracking Functions------------------------*/
/*---------------------------------------------------------------------------*/
gz4d::Point<double> Window3D::getCursorPosOnSurface(GLboolean allowRedisplay)
{
    if (surfaceCursorPosValid)
        return surfaceCursorPos;

    if (cursorValid)
    {
        InputInfo info;
        info.x.i = cursorX;
        info.y.i = cursorY;
    

        surfaceCursorPosValid = getSelectedLocation(&info, surfaceCursorPos, NULL, allowRedisplay);
        if (surfaceCursorPosValid)
            return surfaceCursorPos;
        else
            return gz4d::Point<double>::Invalid();
    }
    return gz4d::Point<double>::Invalid();
}

/*---------------------------------------------------------------------------*/
GLboolean Window3D::getCursorPosOnPlane(double &x, double &y, double z)
{
    double t;
    gz4d::Point<double> screenPoint;

    if (cursorPosAtZValid != GL_TRUE)
    {
        if (!cursorValid || cursorPosAtZValid < 0)
            return GL_FALSE;

        //exactly track the mouse cursor in the indicated z-plane by
        //projecting two points under the cursor, then inserting
        //the z-value to get the intersection with the z-plane.
        //Doing it this way instead of using the eyePoint makes it
        //work for orthogonal projections rather than just perspective.
        screenPoint[X] = (double)cursorX;
        screenPoint[Y] = (double)cursorY;
        screenPoint[Z] = 0.0;
        currentView->getWorldPoint(screenPoint, cursorPosAtScreen);
        screenPoint[Z] = 0.25;
        currentView->getWorldPoint(screenPoint, cursorPosUnderScreen);

        //we have our line info, prepare for finding the parametric
        //t-value at which our line has the value later given for Z.
        zDiffCursor = cursorPosAtScreen[Z] - cursorPosUnderScreen[Z];
        if (zDiffCursor > -0.000001 && zDiffCursor < 0.000001) //too close to zero!
        {
            cursorPosAtZValid = -1;
            return GL_FALSE;
        }
        else
            cursorPosAtZValid = GL_TRUE;
    }

    //cursorPosAtZValid is GL_TRUE here
    t = (z - cursorPosUnderScreen[Z]) / zDiffCursor;
    x = (cursorPosAtScreen[X] - cursorPosUnderScreen[X]) * t + cursorPosUnderScreen[X];
    y = (cursorPosAtScreen[Y] - cursorPosUnderScreen[Y]) * t + cursorPosUnderScreen[Y];

    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
gz4d::Point<double> Window3D::getCursorPosOnSurfaceOrPlane(double z, GLboolean allowRedisplay)
{
    gz4d::Point<double> wPos = getCursorPosOnSurface(allowRedisplay);
    if (!wPos.IsValid())
    {
        cursorPosAtZ[Z] = z;
        if (!getCursorPosOnPlane(cursorPosAtZ[X], cursorPosAtZ[Y], cursorPosAtZ[Z]))
            return gz4d::Point<double>::Invalid();
        wPos = cursorPosAtZ;
    }
    return wPos;
}

/*-------------------------Subwindow Support---------------------------------*/
/*---------------------------------------------------------------------------*/
/** This function renders a background-colored rectangle at the back edge of
*   the screen in order to blank out any leftover artifacts.
*/
void Window3D::BlankLowResResidue()
{
    gz4d::Point<double> worldPos;
    GLint depthFunc;

    gz4d::Point<double> screenPos(0.0,0.0,0.9999);

    //0,0
    currentView->getWorldPoint(screenPos, worldPos);
    //the above line needs to be out here, above the Depth
    //functions, I'm assuming.  Otherwise it messes up the
    //matrix calculations in getWorldPoint.

    glColor4f(bgColor[0], bgColor[1], bgColor[2], 1.0);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glBegin(GL_QUADS);
    //0,0
    gz4d::glo::Vertex(worldPos);
    //width, 0
    screenPos[X] = (float) width;
    currentView->getWorldPoint(screenPos, worldPos);
    gz4d::glo::Vertex(worldPos);
    //width, height
    screenPos[Y] = (float) height;
    currentView->getWorldPoint(screenPos, worldPos);
    gz4d::glo::Vertex(worldPos);
    //0, height
    screenPos[X] = 0.0;
    currentView->getWorldPoint(screenPos, worldPos);
    gz4d::glo::Vertex(worldPos);
    glEnd();

    glDepthFunc(depthFunc);
    glDepthMask(GL_TRUE);
}

/*---------------------------------------------------------------------------*/
/**
*   Adds the passed Window3D to the list of managed subwindows.  If the
*   argument is NULL, a new SubWindow is created.
*/
void Window3D::AddSubwindow(SubWindow *subwin)
{
    Node *newSubwinNode;

    if (!subwin)
        subwin = new SubWindow(this);

    //Create a new node, and tack it on the end of the list.
    newSubwinNode = new Node;
    newSubwinNode->window = subwin;
    newSubwinNode->next = NULL;
    newSubwinNode->prev = subWindowTail;
    if (subWindowTail)
        subWindowTail->next = newSubwinNode;
    if (!subWindows)
        subWindows = newSubwinNode;
    subWindowTail = newSubwinNode;
    subWindowCount++;
    visibleSubwindowCount++;
}

/*---------------------------------------------------------------------------*/
/**
*   Removes the passed SubWindow from the list of managed subwindows.
*/
void Window3D::RemoveSubwindow(SubWindow *subwin)
{
    Node *currentNode;

    if (subwin == overviewWindow)
        overviewWindow = NULL;

    for (currentNode = subWindows; currentNode != NULL; currentNode = currentNode->next)
    {
        if (currentNode->window == subwin)
        {
            if (currentNode->prev == NULL)
                subWindows = currentNode->next;
            else
                currentNode->prev->next = currentNode->next;

            if (currentNode->next == NULL)
                subWindowTail = currentNode->prev;
            else
                currentNode->next->prev = currentNode->prev;

            subWindowCount--;
            delete currentNode;
            return;
        }
    }
}

/*---------------------------------------------------------------------------*/
/**
*   This procedure draws its subwindow proxies after all the scene objects
*   have been drawn.
*/
void Window3D::DrawSubwindows(long gl_side)
{
    if (stereo_flag == 1)
    {
        if (gl_side == GL_FRONT)
        {
            DrawSubwindows(GL_FRONT_LEFT);
            DrawSubwindows(GL_FRONT_RIGHT);
            return;
        }
        else if (gl_side == GL_BACK)
        {
            DrawSubwindows(GL_BACK_LEFT);
            DrawSubwindows(GL_BACK_RIGHT);
            return;
        }
    }

    glDisable(GL_LIGHTING);
    PushTransform(gl_side);
    if (gl_side == GL_FRONT || gl_side == GL_FRONT_LEFT
        || gl_side == GL_FRONT_RIGHT)
        BlankLowResResidue();
    PerformDrawIns();
    PopTransform();
    glEnable(GL_LIGHTING);
    glFlush();
}

/*---------------------------------------------------------------------------*/
/** This procedure iterates through, drawing all the subwindow proxies for
*   the currently set view.  This virtual function allows child classes to
*   define their own lists to be rendered.  Eventually, this might end up
*   being a more general transparency-layer drawing routine.
*/
void Window3D::PerformDrawIns()
{
    Node *currentNode;

    for (currentNode = subWindows; currentNode; currentNode = currentNode->next)
        currentNode->window->DrawIn(this);
}

/*---------------------------------------------------------------------------*/
/**
*   This function restores this window's overview window, or creates it if
*   it does not exist.
*/
int Window3D::ShowOverviewWindow()
{
    Node *currentNode;

    if (!overviewWindow)
    {
        overviewWindow = new SubWindow(this, GL_TRUE, getAddress()->getName());
        AddSubwindow(overviewWindow);
    }
    else
    {
        for (currentNode = subWindows; currentNode; currentNode = currentNode->next)
        {
            if (currentNode->window == overviewWindow)
            {
                currentNode->window->SetVisible(GL_TRUE);
                return TCL_OK;
            }
        }
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function restores all hidden windows.
*/
int Window3D::RestoreSubwindows()
{
    Node *currentNode;

    for (currentNode = subWindows; currentNode; currentNode = currentNode->next)
        currentNode->window->SetVisible(GL_TRUE);

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function registers a change in the hidden/visible state of a subwindow
*/
void Window3D::registerSubWindowVisibility(GLboolean visibility)
{
    if (visibility)
        visibleSubwindowCount++;
    else
        visibleSubwindowCount--;
}

/*------------------------Rendering for Proxy--------------------------------*/
/*---------------------------------------------------------------------------*/
/**
*   This function draws its proxy in the caller's context.
*/
void Window3D::DrawIn(Window3D *inWindow)
{
    currentView->DrawIn(inWindow);
}

/*---------------------------------------------------------------------------*/
/**
*   This function adds a SubWindow request the Scene3D's stack
*/
void Window3D::requestNewSubwindow(Object * creator, Window3D * parentWindow)
{
    scene3d->requestNewSubwindow(creator, parentWindow);
}

int Window3D::VideoRecorderCommand(int argc, const char *argv[])
{
    if (argc > 0)
    {
        std::string command(argv[0]);
        if (command == "start")
            return StartVideoRecorder( argc, argv);
        else if (command == "stop")
            return StopVideoRecorder( argc, argv);
        else if (command == "addFrame")
            return AddFrame( argc, argv);
        else if (command == "recordFrame")
            return RecordFrame( argc, argv);
        else if (command == "set")
        {
            if (vrec->setParameter( argc, argv))
                return TCL_OK;
            else
                return TCL_ERROR;
        }
        else if (command == "get")
        {
            Tcl_SetResult(interp, (char*)vrec->getParameter(argc,argv).c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
/*        else if (command == "getFormats")
        {
            Tcl_SetResult(interp, (char*)VideoRecorder::getFormats().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        else if (command == "getCodecs")
        {
            Tcl_SetResult(interp, (char*)VideoRecorder::getCodecs().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }*/
    }
    return TCL_ERROR;
}

int Window3D::StartVideoRecorder(int argc, const char *argv[])
{

    if(!establishValidContext())
        return TCL_ERROR;

    auto activeToken = getGloContext()->activate();

    if(!vrec->Init())
            return TCL_ERROR;

    Scene3D::timeControl->setRecording();
    //glClearColor(0.0, 0.0, 0.0, 0.0);
    otherMovieWindow = NULL;

    if(argc > 2)
    {
        std::string withCheck(argv[1]);
        if (withCheck == std::string("with"))
        {
            std::string withWin(argv[2]);
            if (withWin == std::string("time"))
            {
                otherMovieWindow = Scene3D::timeControl;
                otherMovieWindowIsAbove = false;
            }
            else
            {
                Addressable *address = NULL;
                Window3D *win = NULL;
                gzCommandInterface::findWindow(withWin, address, win);
                otherMovieWindow = win;
                otherMovieWindowIsAbove = true;
            }
        }
    }

    return TCL_OK;
}

int Window3D::StopVideoRecorder(int argc, const char *argv[])
{
    if(!establishValidContext())
        return TCL_ERROR;
    vrec->Done();
    Scene3D::timeControl->setRecording(false);

        //glClearColor(bgColor[0], bgColor[1], bgColor[2], 0.0);

    return TCL_OK;
}

int Window3D::AddFrame(int argc, const char *argv[])
{
    int h, vh, vw;
    if(!establishValidContext())
        return TCL_ERROR;

    auto activeToken = getGloContext()->activate();

    vrec->bind();
    vrec->setRecordedFrame();
        vh = vrec->getHeight();
        vw = vrec->getWidth();
        if (otherMovieWindow)
        {
                h = otherMovieWindow->getHeight();
                otherMovieWindow->Reshape(0, (otherMovieWindowIsAbove? (vh - h) : 0), vw, h);
                otherMovieWindow->Display();
                if (otherMovieWindowIsAbove)
                        vrec->AddFrame(atof(argv[1]), vh - h, vh, false);
                else
                        vrec->AddFrame(atof(argv[1]), 0, h, false);
        }
        else
                h = 0;
    Reshape(0, (otherMovieWindowIsAbove? 0 : h), vw, vh - h);
    Display();
/*  if (watermark && vrec->useWatermark())
    {
        GLint df;
        glGetIntegerv(GL_DEPTH_FUNC,&df);
        glDepthFunc(GL_ALWAYS);
        beginRaster2D(1.0 - (2.0*watermark->GetWidth()/float(width)), -1.0);
        watermark->Draw();
        endRaster2D();
        glDepthFunc(df);
    }*/
    if (otherMovieWindowIsAbove)
            vrec->AddFrame(atof(argv[1]), 0, vh - h);
    else
            vrec->AddFrame(atof(argv[1]), h, vh);
    vrec->unbind();
    vrec->setRecordedFrame(false);
        if (otherMovieWindow)
                otherMovieWindow->Reshape();
    Reshape();
    return TCL_OK;
}

int Window3D::RecordFrame(int argc, const char *argv[])
{
    vrec->WriteFrame();
    return TCL_OK;
}

bool Window3D::isRecording()
{
    return (vrec->isRecordedFrame());
}

void Window3D::ScreenShot(int w, int h, fs::path fname)
{
    establishValidContext();
    gz4d::glo::Framebuffer osb(context);

    try
    {
        osb.Bind();
        gz4d::glo::Renderbuffer crb(context,GL_RGB8);
        crb.Bind();
        crb.Storage(w,h);
        osb.Attach(crb);

        gz4d::glo::Renderbuffer drb(context,GL_DEPTH_COMPONENT32);
        drb.Bind();
        drb.Storage(w,h);
        osb.Attach(drb);

        osb.CheckStatus();

        Scene3D::timeControl->setRecording();

        vrec->setRecordedFrame();
        Reshape(0,  0, w, h);
        Display();

        gz4d::Raster<gz4d::Pixel<gz4d::RGB8> >::Ptr r(new gz4d::Raster<gz4d::Pixel<gz4d::RGB8> >);
        r->Resize(w,h);
        gz4d::ImageRGB8 img(r);
        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
        gz4d::glo::ReadPixels(*r);
        osb.Unbind();
        vrec->setRecordedFrame(false);
        Reshape();
        Scene3D::timeControl->setRecording(false);
        img.Save(fname.string());
    }
    catch (gz4d::Exception e)
    {
        std::cerr << "Screenshot failed: " << e.what() << std::endl;
    }
}

bool Window3D::globalPrintAt(double x, double y, double z, char *string, GLboolean shadow, void *font, int xOffset, int yOffset, GLboolean atScreenDepth)
{
    gz4d::Point<double> r = getViewInfo()->getPosable()->getPosition();

    return ToglWindow::printAt(x - r[X],y - r[Y],z - r[Z],string,shadow,font,xOffset,yOffset,atScreenDepth);
}
