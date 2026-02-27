/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// SubWindow.h
//
//  This class implements a subwindow (child) viewport Window3D.
//  Recreated by mdp 7/27/2000
/*---------------------------------------------------------------------------*/

#include "SubWindow.h"
#include <stdlib.h>
#include "engine/PosableStateBase.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/ViewManager.h"
#include "tclLinkage.h"
#include "widgets/Sv_Standards.h"
#include "interactors/Pickable.h"
#include "utilities/DottedAddress.h"
#include "engine/Scene3D.h"
#include "engine/SDV_Light.h"
#include <string.h>

int SubWindow::totalSubwindowCount = 0;
int SubWindow::subwindowToglInitialized = 0;
int SubWindow::subwindowsBehavior = SUBWINDOWS_FOLLOW_PARENT;
/*
int SubWindow::subwindowsBehavior = SUBWINDOWS_CLIPPED_BY_PARENT;
int SubWindow::subwindowsBehavior = SUBWINDOWS_SEPARATE;
*/
#define MAXCOMMAND 256

/*---------------------------------------------------------------------------*/
/** Generic constructor.  Minimal work done to get up a subwindow.  If the
 *  overviewFlag is set, creates the subwindow as an overview window.
 */
SubWindow::SubWindow(Window3D *parentWin, GLboolean overviewFlag, std::string name)
:Window3D(NULL, parentWin)
{
    overview = overviewFlag;
    lightingLock = NULL;
    construct(name);
}

/*---------------------------------------------------------------------------*/
/** Auto-constructor.  Creates a subwindow at the passed center and scale,
 *  while keeping the orientation of the parent window.  If no scale is given,
 *  the parent's scale is used.  If center is NULL, the parent's center is
 *  used.
 */
SubWindow::SubWindow(Window3D *parentWin, gz4d::Point<double> const &center, GLdouble scale, std::string name)
:Window3D("", parentWin)
{
    overview = GL_FALSE;
    *currentView = *parent->getViewInfo();

    construct(name);

    if (center.IsValid())
    {
        currentPose->setPosition((float)center[X], (float)center[Y], (float)center[Z]);
    }
    if (scale != 0.0)
    {
        currentPose->setScale((float)scale);
    }
    currentPose->setPoseInfoFromTo(ChangePose, ChangeHomePose);
}

/*---------------------------------------------------------------------------*/
/** Common construction code.
 */
void SubWindow::construct(std::string name)
{
    std::stringstream command;
    char *parentTogl;
    std::stringstream myName;
    
    visible = true;
    proxyEnabled = !overview;
    setScene3D(parent->getScene3D());

    if (name.empty())
        myName << "sub" << totalSubwindowCount;
    else
        {
            myName << name;
            if (overview)
                myName << "Overview";
        }

    Tcl_SetResult(interp, (char *)(myName.str().c_str()), TCL_VOLATILE);

    publishName(myName.str()); //me as an interactor
    getAddress()->reparent(InteractorWindowNeighborhood);
    currentPose->publishName(myName.str()); //my posable
    inputMediator.publishName(myName.str()); //my input mediator
    InteractorNeighborhood->addChild(getAddress());
    WindowNeighborhood->addChild(currentPose->getAddress());

    //create the subwindow
    parentTogl = Togl_Ident(parent->GetTogl());
    if (subwindowsBehavior == SUBWINDOWS_CLIPPED_BY_PARENT)
        command << "subtogl $toglPath(" << parentTogl << ")." << myName.str() << (overview? " -overview 1 " : " ") << "-parent " << parentTogl << "-double true -depth true";
    else
        command << "subtogl ." << myName.str() << " " << (overview? "-overview 1" : "") << " -parent " << parentTogl << " -width 380 -height 285 -double true -depth true";

    parent->Tcl_Eval(command.str().c_str());

    setParent = lockRotateX = lockRotateY = lockScale = lockPosition = GL_FALSE;
    currentView->lockTo(parent->getViewInfo());

    if (!proxyEnabled)
        currentView->setVisible(GL_FALSE);

    if (overview)
    {
        currentView->lock(PosableStateLock::LockTranslate);
        lockPosition = GL_TRUE;
        currentPose->setOrientation(90.0, 0.0);
        currentPose->setScale(.2f * parent->getPoseInfo()->getScale());
        currentPose->setPoseInfoFromTo(ChangePose, ChangeHomePose);
    }
}

/*---------------------------------------------------------------------------*/
SubWindow::~SubWindow()
{
    currentView->cancelAnchor(parent);
    if (visible)
        parent->registerSubWindowVisibility(GL_FALSE);
    parent->RemoveSubwindow(this);
    parent->inputMediator.Trigger(windowChangedInputType->value);
}

/*---------------------------------------------------------------------------*/
/**
 *  Allows for child-specific initialization, sets up the Tcl interpreter if
 *  it is not already set up.
 */
void SubWindow::Init(int argc, char *argv[])
{
    const char *newCommand[] = {"minimize", "positionUpdated", "restore",
                          "view_set", "view_lock_scale", "view_lock_position",
                          "view_lock_rotate_x", "view_lock_rotate_y",
                          "view_set_scale", "view_get_scale",
                          "view_set_position", "view_get_position",
                          "view_set_rotate_x", "view_get_rotate_x",
                          "view_set_rotate_y", "view_get_rotate_y",
                          "lighting",
                          NULL};
    int currentCommand;

    Window3D::Init(argc, argv);

    // Initialize Togl commands for objects
    if (!subwindowToglInitialized)
    {
        Tcl_LinkVar("subwindowsBehavior", (char *)&subwindowsBehavior);
        Tcl_LinkVar("subWindowCount", (char *)&totalSubwindowCount);
        for (currentCommand = 0; newCommand[currentCommand]; currentCommand++)
        Togl_CreateCommand(newCommand[currentCommand], SubCommandCallBack);
        subwindowToglInitialized = 1;
    }

    lightingLock = new PosableStateLock(this, getLight()->getPosable(), parent->getLight()->getPosable(), PosableStateLock::LockTranslate | PosableStateLock::LockBoth);
}

/*-----------------------------Accessors-------------------------------------*/
/*---------------------------------------------------------------------------*/
/** Returns the X position of the window relative to the parent window
 */
int SubWindow::getX(GLboolean relativeToMain)
{
    if (subwindowsBehavior & SUBWINDOWS_FOLLOW_PARENT)
        if (relativeToMain)
            return xLocation + parent->getX(relativeToMain);
        else
            return xLocation;
    else
        if (relativeToMain)
            return xLocation;
        else
            return xLocation - parent->getX(relativeToMain);
}

/*---------------------------------------------------------------------------*/
/** Returns the Y position of the window relative to the parent window
 */
int SubWindow::getY(GLboolean relativeToMain)
{
    if (subwindowsBehavior & SUBWINDOWS_FOLLOW_PARENT)
        if (relativeToMain)
            return yLocation + parent->getY(relativeToMain);
        else
            return yLocation;
    else
        if (relativeToMain)
            return yLocation;
        else
            return yLocation - parent->getY(relativeToMain);
}

/*---------------------------------------------------------------------------*/
/** Returns the full width of the subwindow, including borders.
 */
int SubWindow::getOuterWidth()
{
    return xSize;
}

/*---------------------------------------------------------------------------*/
/** Returns the full height of the subwindow, including borders.
 */
int SubWindow::getOuterHeight()
{
    return ySize;
}

/*----------------------------Input Callbacks--------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  This function is the branch-point for Togl subcommands defined by
 *  ToglWindow subclasses.  All subclasses must call the parent class's
 *  SubCommand procedure after they have exhausted the subcommand
 *  possibilities in their implementation of this procedure.
 *
 *  @param argc Count of argv elements
 *  @param argv
 *  argv[0]     Togl object name
 *  argv[1]     Togl subcommand name (known to be registered)
 *  argv[2...]  Arguments
 *  @return             TCL_OK / TCL_ERROR
 */
int SubWindow::SubCommand(int argc, char *argv[])
{
    if (!strcmp(argv[1], "minimize"))
        return SetVisible(GL_FALSE);
    if (!strcmp(argv[1], "restore"))
        return SetVisible(GL_TRUE);
    if (!strcmp(argv[1], "positionUpdated"))
        return PositionUpdated(argc, argv);
    if (!strcmp(argv[1], "view_set"))
        return SettingParent(atoi(argv[2]));
    if (!strcmp(argv[1], "view_lock_scale"))
        return LockScale(atoi(argv[2]));
    if (!strcmp(argv[1], "view_lock_position"))
        return LockPosition(atoi(argv[2]));
    if (!strcmp(argv[1], "view_lock_rotate_x"))
        return LockRotate(atoi(argv[2]), X);
    if (!strcmp(argv[1], "view_lock_rotate_y"))
        return LockRotate(atoi(argv[2]), Y);
    if (!strncmp(argv[1], "view_set_", 9))
        return ViewAttribute(&argv[1][9], SetView);
    if (!strncmp(argv[1], "view_get_", 9))
        return ViewAttribute(&argv[1][9], GetView);
    if (!strcmp(argv[1], "lighting"))
    {
        if (!strcmp(argv[2], "unlock"))
            lightingLock->unlock(PosableStateLock::LockTranslate);
        else
            lightingLock->lock(PosableStateLock::LockTranslate);
        return TCL_OK;
    }

    return Window3D::SubCommand(argc, argv);
}

/*---------------------------------------------------------------------------*/
/** This function updates the internal knowledge of this window's location
 *  relative to its parent.
 */
int SubWindow::PositionUpdated(int argc, char *argv[])
{
    if (argc >= 4)
    {
        xLocation = atoi(argv[2]);
        yLocation = atoi(argv[3]);
    }
    if (argc >= 5)
        xSize = atoi(argv[4]);
    if (argc >= 6)
        ySize = atoi(argv[5]);

    parent->inputMediator.Trigger(windowChangedInputType->value);
    if (ViewManager::viewMovesWithWindow && argc < 5)
        Display();
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
 *  This function posts the appropriate option menu for this SubWindow.  It
 *  makes certain to disable non-sensible options (such as restore subwindows
 *  when none need to be restored).
 */
void SubWindow::PostOptionMenu(int x, int y)
{
    char command[MAXCOMMAND];
    GLboolean lightingLocked;

    if ((lightingLock->getType() & PosableStateLock::LockTranslate) == 0)
        lightingLocked = GL_FALSE;
    else
        lightingLocked = GL_TRUE;

    sprintf(command, "postSubToglMenu %s %d %d", Togl_Ident(myTogl), x, y);
    strcat(command, " {entryconfigure {" LABEL_RESTORE_SUBS);
    if (subWindows && visibleSubwindowCount != subWindowCount)
        strcat(command, "} -state active}");
    else
        strcat(command, "} -state disabled}");
    strcat(command, " {entryconfigure {" LABEL_LOCK_LIGHTING);
    if (lightingLocked)
        strcat(command, "} -state disabled}");
    else
        strcat(command, "} -state active}");
    strcat(command, " {entryconfigure {" LABEL_UNLOCK_LIGHTING);
    if (lightingLocked)
        strcat(command, "} -state active}");
    else
        strcat(command, "} -state disabled}");
    Tcl_Eval(command);
}

/*---------------------------------------------------------------------------*/
/**
 *  Processes events targeted for this SubWindow.
 *
 *  @param  inputType   Type field of the info structure
 *  @param  info        Full event info structure
 *  @return             0=not handled / 1=handled / -1=handled,remove
 */
int SubWindow::HandleInput(unsigned int inputType, InputInfo *info)
{
    if (inputType & EnumeratedAction)
    {
        if (inputType == currentView->viewChangedEvent()) //mdp added case 5/4/2001
        {
            Window3D::HandleInput(inputType, info);
            parent->inputMediator.Trigger(windowChangedInputType->value);
            return SINK_EVENT;
        }
    }
    else
    {
        if (inputType & KeyStroke)
        {   switch (info->val.i)
            {   
                case 'h': // restore the home position
                    Window3D::HandleInput(inputType, info);
                    parent->inputMediator.Trigger(windowChangedInputType->value);
                    return SINK_EVENT;
            }
        }
    }
    return Window3D::HandleInput(inputType, info);
}

/*---------------------------------------------------------------------------*/
/**
 *  This function extends the idle-time display behavior of the Window3D
 *  class by telling the parent window that a child instance has changed.
 */
void SubWindow::Idle()
{
    Window3D::Idle();

    if (currentPose->isInContinuousChange() && parent)
        parent->inputMediator.Trigger(windowChangedInputType->value);
}

/*---------------------------------------------------------------------------*/
/**
 *  This function tells the parent window that a child has changed.
 */
void SubWindow::Reshape(int x, int y, int w, int h)
{
    Window3D::Reshape(x,y,w,h);
    if (parent)
        parent->inputMediator.Trigger(windowChangedInputType->value);
}

/*-------------------------------Visibility----------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  This function returns whether or not the subwindow is on posted to the
 *  screen.
 */
GLboolean SubWindow::IsVisible()
{
    return visible;
}

/*---------------------------------------------------------------------------*/
/**
 *  This function sets the visibility flag of this subwindow.
 *
 *  @return             TCL_OK / TCL_ERROR
 */
int SubWindow::SetVisible(GLboolean vis)
{
    char command[MAXCOMMAND];

    if (vis == visible)
        return TCL_OK;

    if (!visible)
        sprintf(command, "subwindowRestore %s", Togl_Ident(GetTogl()));
    else
        sprintf(command, "subwindowMinimize %s", Togl_Ident(GetTogl()));
    if (Tcl_Eval(command) != TCL_OK)
        return TCL_ERROR;

    visible = vis;
    parent->registerSubWindowVisibility(visible);
    parent->inputMediator.Trigger(windowChangedInputType->value);
    currentView->setVisible(GL_TRUE);
    currentView->setMinimized(!vis);
    return TCL_OK;
}

/*------------------------Rendering for Proxy--------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  This function draws its proxy in the caller's context, unless we are an
 *  overview window.
 */
void SubWindow::DrawIn(Window3D *inWindow)
{
    if (proxyEnabled)
        Window3D::DrawIn(inWindow);
}

/*---------------------------------------------------------------------------*/
/** This procedure defaults to Window3D rendering of subwindows, plus if we
 *  are an overview window, we draw our parent's window if
 *  we are not locked to the parent's translate.
 */
void SubWindow::PerformDrawIns()
{
    Window3D::PerformDrawIns();
    if (overview && !lockPosition)
        parent->DrawIn(this);
}

/*---------------------------------------------------------------------------*/
/**
 *  Called whenever a picking round is beginning.  Activates the parent's
 *  WindowAnchor for picking, if this is an overview window that is not
 *  locked in translation.  Also defaults to parent's picking.
 */
void SubWindow::startPickingRound(int pickType)
{
    Window3D::startPickingRound(pickType);
    if (pickType == Pickable::PICK_PRESS1 && overview && !lockPosition)
        parent->enablePicking();
}

void SubWindow::enableProxy(GLboolean show)
{
    proxyEnabled = show;
    currentView->setVisible(proxyEnabled);
}

/*----------------------------View Locking Procs-----------------------------*/
/*---------------------------------------------------------------------------*/
/** Changes whether we set the parent from our view when we lock, or vice versa
 */
int SubWindow::SettingParent(GLboolean state)
{
    if (state == setParent)
        return TCL_OK;

    if (state)
        currentView->lock(PosableStateLock::LockSetFromOwner);
    else
        currentView->unlock(PosableStateLock::LockSetFromOwner);

    setParent = state;

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** Should our view's scale be locked to our parent?
 */
int SubWindow::LockScale(GLboolean state)
{
    if (state == lockScale)
        return TCL_OK;

    if (state)
        currentView->lock(PosableStateLock::LockDeltaScale);
    else
        currentView->unlock(PosableStateLock::LockDeltaScale);

    lockScale = state;

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** Should our view's position be locked to our parent?
 */
int SubWindow::LockPosition(GLboolean state)
{
    long lockType;

    if (state == lockPosition)
        return TCL_OK;

    lockType = PosableStateLock::LockTranslate;
    //lockType = PosableStateLock::LockDeltaTranslate;

    if (state)
        currentView->lock(lockType);
    else
        currentView->unlock(lockType);

    lockPosition = state;

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** Should our view's rotation be locked to our parent?
 */
int SubWindow::LockRotate(GLboolean state, int direction)
{
    long lockType;

    if ((direction == X && state == lockRotateX) ||
        (direction == Y && state == lockRotateY))
        return TCL_OK;

    if (direction == X)
    {
        lockType = PosableStateLock::LockDeltaRotateX;
        lockRotateX = state;
    }
    else if (direction == Y)
    {
        lockType = PosableStateLock::LockDeltaRotateY;
        lockRotateY = state;
    }
    else
        return TCL_ERROR;

    if (state)
        currentView->lock(lockType);
    else
        currentView->unlock(lockType);


    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** Set or get parent's attrib view parameter (set uses my attribute to set
 *  my parent's, get sets my attribute according to my parent's).
 */
int SubWindow::ViewAttribute(const char *attrib, ViewAction action)
{
    long viewType;

    if (!strcmp(attrib, "scale"))
        viewType = PosableStateLock::LockScale;
    else if (!strcmp(attrib, "position"))
        viewType = PosableStateLock::LockTranslate;
    else if (!strcmp(attrib, "rotate_x"))
        viewType = PosableStateLock::LockRotateX;
    else if (!strcmp(attrib, "rotate_y"))
        viewType = PosableStateLock::LockRotateY;
    else
        return TCL_ERROR;

    if (action == SetView)
        viewType |= PosableStateLock::LockFromOwner;
    currentView->setView(viewType);

    return TCL_OK;
}
