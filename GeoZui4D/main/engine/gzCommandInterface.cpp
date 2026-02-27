/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// gzCommandInterface.cpp
//
//  This class defines the Tcl commands specific to GeoZiu3D.
//
// Created by mdp 12/7/2001
///////////////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include "engine/PosableStateBase.h"
#include "engine/Object.h"
#include "engine/SDV_Light.h"
#include "engine/Scene3D.h"
#include <tcl.h>
#include <tk.h>
#include <gz4d/glo.h>
#include <GL/glu.h>
#include <stdlib.h>
#include "engine/ViewManager.h"
#include "utilities/DottedAddress.h"
#include "utilities/Addressable.h"
#include "SubWindow.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/gzCommandInterface.h"
#include "engine/ConversionGrid.h"
#include <string.h>
#include <float.h>
#include <typeinfo>
#include "engine/ObjectPoseState.h"
#include "engine/TranslatableState.h"
#include "engine/WindowPoseState.h"
#include "engine/ExaggeratableState.h"
#include "interactors/Animator.h"
#include "interactors/ImitatingAnimator.h"
#include "interactors/PathAnimator.h"
#include "interactors/VectorAnimator.h"
#include "engine/TranslatableWrapper.h"
//#include "engine/Joystick.h"
#include "interactors/EventHandler.h"
#include <boost/filesystem.hpp>
#include "interactors/InputTentacle.h"
#include "engine/ClutEditor.h"
#include "objects/DynamicTides.h"
#include "objects/gutm.h"
#include <sstream>
#include <iomanip>
#include "utilities/Utilities.h"
#include "engine/TimeControl.h"
#include <gz4d/geo/Projection.h>

#ifdef WIN32
#ifdef HAVE_FREEIMAGE
#include <windows.h>
#include <FreeImagePlus.h>
#endif
#endif

#ifdef EYE_TRACKER
#include "../EyeScience/EyeScienceSDK.h"
#endif

#define MAXCOMMAND 256
#define GZ_VERSION "1.0"

#include "tclLinkage.h"

#ifdef _WIN32
LINK_ANIMATOR(ImitatingAnimator);
LINK_ANIMATOR(PathAnimator);
LINK_ANIMATOR(VectorAnimator);
#endif

extern MainWindow *mainWindow;
//extern Joystick *joystickEventMediator;

static int Gz_Cmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, const char **argv);
Tcl_Interp *gzCommandInterface::interp = NULL;
long gzCommandInterface::uniqueCount = 0;
DynamicTides *gzCommandInterface::dTides = NULL;

using namespace std;

#ifdef WIN32
static map<string, int> keyValues;
#endif

/*-----------------------------Initialization--------------------------------*/
/*---------------------------------------------------------------------------*/
gzCommandInterface::gzCommandInterface()
{
    scene3d = NULL;
    mainwin = mainWindow;

#ifdef WIN32
#define RELEASE_KEYS -1
#define HOLD_KEYS -2
    if (keyValues.size() <= 0)
    {
        keyValues["Release:"] = RELEASE_KEYS;
        keyValues[":Hold"] = HOLD_KEYS;

        keyValues["Alt"] = VK_MENU;
        keyValues["Ctrl"] = VK_CONTROL;
        keyValues["Shift"] = VK_SHIFT;

        keyValues["Windows"] = VK_LWIN;
        keyValues["WindowsRight"] = VK_RWIN;

        keyValues["Backspace"] = VK_BACK;
        keyValues["Escape"] = VK_ESCAPE;
        keyValues["Tab"] = VK_TAB;

        keyValues["PageUp"] = VK_PRIOR;
        keyValues["PageDown"] = VK_NEXT;
        keyValues["End"] = VK_END;
        keyValues["Home"] = VK_HOME;
        keyValues["Left"] = VK_LEFT;
        keyValues["Right"] = VK_RIGHT;
        keyValues["Up"] = VK_UP;
        keyValues["Down"] = VK_DOWN;
        keyValues["Ins"] = VK_INSERT;
        keyValues["Del"] = VK_DELETE;

        keyValues["PrintScreen"] = VK_SNAPSHOT;
        keyValues["Pause"] = VK_PAUSE;
        keyValues["ScrollLock"] = VK_SCROLL;
        keyValues["NumLock"] = VK_NUMLOCK;
        keyValues["CapsLock"] = VK_CAPITAL;

        keyValues["Space"] = VK_SPACE;

        keyValues["Select"] = VK_SELECT;
        keyValues["Print"] = VK_PRINT;
        keyValues["Execute"] = VK_EXECUTE;
        keyValues["Help"] = VK_HELP;
        keyValues["F1"] = VK_F1;
        keyValues["F2"] = VK_F2;
        keyValues["F3"] = VK_F3;
        keyValues["F4"] = VK_F4;
        keyValues["F5"] = VK_F5;
        keyValues["F6"] = VK_F6;
        keyValues["F7"] = VK_F7;
        keyValues["F8"] = VK_F8;
        keyValues["F9"] = VK_F9;
        keyValues["F10"] = VK_F10;
        keyValues["F11"] = VK_F11;
        keyValues["F12"] = VK_F12;
        keyValues["F13"] = VK_F13;
        keyValues["F14"] = VK_F14;
        keyValues["F15"] = VK_F15;
        keyValues["F16"] = VK_F16;
        keyValues["F17"] = VK_F17;
        keyValues["F18"] = VK_F18;
        keyValues["F19"] = VK_F19;
        keyValues["F20"] = VK_F20;
        keyValues["F21"] = VK_F21;
        keyValues["F22"] = VK_F22;
        keyValues["F23"] = VK_F23;
        keyValues["F24"] = VK_F24;
        keyValues["0"] = 0x30;
        keyValues["1"] = 0x31;
        keyValues["2"] = 0x32;
        keyValues["3"] = 0x33;
        keyValues["4"] = 0x34;
        keyValues["5"] = 0x35;
        keyValues["6"] = 0x36;
        keyValues["7"] = 0x37;
        keyValues["8"] = 0x38;
        keyValues["9"] = 0x39;
        keyValues["a"] = 0x41;
        keyValues["b"] = 0x42;
        keyValues["c"] = 0x43;
        keyValues["d"] = 0x44;
        keyValues["e"] = 0x45;
        keyValues["f"] = 0x46;
        keyValues["g"] = 0x47;
        keyValues["h"] = 0x48;
        keyValues["i"] = 0x49;
        keyValues["j"] = 0x4A;
        keyValues["k"] = 0x4B;
        keyValues["l"] = 0x4C;
        keyValues["m"] = 0x4D;
        keyValues["n"] = 0x4E;
        keyValues["o"] = 0x4F;
        keyValues["p"] = 0x50;
        keyValues["q"] = 0x51;
        keyValues["r"] = 0x52;
        keyValues["s"] = 0x53;
        keyValues["t"] = 0x54;
        keyValues["u"] = 0x55;
        keyValues["v"] = 0x56;
        keyValues["w"] = 0x57;
        keyValues["x"] = 0x58;
        keyValues["y"] = 0x59;
        keyValues["z"] = 0x5A;
    }
#endif
}

/*---------------------------------------------------------------------------*/
gzCommandInterface::~gzCommandInterface()
{
}

/*---------------------------------------------------------------------------*/
/*
* Gz_Init
*
*   Called upon system startup to create Togl command.
*/
int gzCommandInterface::Gz_Init(Tcl_Interp *interpreter)
{
    interp = interpreter;
    if (Tcl_PkgProvide(interp, "Gz", GZ_VERSION) != TCL_OK) {
        return TCL_ERROR;
    }

    Tcl_CreateCommand(interp, "gz", Gz_Cmd,(ClientData) this, NULL);
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
* Gz_Cmd
*
*   Supported subcommands:
*       posable, explainobject, hideobject, showobject, loadobject,
*       saveobject, deleteobject, reloadobject, buildscene, clearScene,
*       getObjectTypes, getObjectExtensions, getobjext, projection,
*       animator, tellObject, tellSelectedObject, address, 
*       setTransparentBlendThreshold (stopwatch partially coded in comments)
*/
static int Gz_Cmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, const char **argv)
{
    int retval;
    float val;
    gzCommandInterface *self = (gzCommandInterface *) clientData;

//     LogState ls(&gzlog,"Gz_Cmd",TRACE);
//     gzlog << argv[0];
//     for (int i=1; i<argc; i++)
//         gzlog << ", " << argv[i];
//     gzlog << endl;


    if (argc > 1)
    {
        if (!strcmp(argv[1], "posable"))
            return self->PosableCommand(argc, argv);
#ifdef EYE_TRACKER
        if (!strcmp(argv[1], "eyetrack"))
            return self->EyeTrackCommand(argc, argv);
#endif
/*      else if (!strcmp(argv[1], "stopwatch"))
            return self->controlStopwatch(argc, argv); */
        else if (!strcmp(argv[1], "explainobject"))
            return self->ExplainObject(argc, argv);
        else if (!strcmp(argv[1], "hideobject"))
            return self->HideObject(argc, argv);
        else if (!strcmp(argv[1], "showobject"))
            return self->ShowObject(argc, argv);
        else if (!strcmp(argv[1], "loadobject"))
            return self->LoadObject(argc, argv);
        else if (!strcmp(argv[1], "newobject"))
            return self->NewObject(argc, argv);
        else if (!strcmp(argv[1], "saveobject"))
            return self->SaveObject(argc, argv);
        else if (!strcmp(argv[1], "saveEdited"))
            return self->SaveEditedObjects(argc, argv);
        else if (!strcmp(argv[1], "deleteobject"))
            return self->DeleteObject(argc, argv);
        else if (!strcmp(argv[1], "reloadobject"))
            return self->ReloadObject(argc, argv);
        else if (!strcmp(argv[1], "buildscene"))
            return self->BuildScene(argc, argv);
        else if (!strcmp(argv[1], "savescene"))
            return self->SaveScene(argc, argv);
        else if (!strcmp(argv[1], "clearScene"))
            return self->ClearScene();
        else if (!strcmp(argv[1], "ping"))  // useful to test GZConsole, etc...
        {
            Tcl_SetResult(interp, const_cast<char*>("pong"), TCL_VOLATILE);
            return TCL_OK;
        }
        else if (!strcmp(argv[1], "getObjectTypes"))
        {
            Tcl_SetResult(interp, (char *) self->GetObjectTypes().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        else if (!strcmp(argv[1], "getObjectExtensions"))
        {
            Tcl_SetResult(interp, (char *) self->GetObjectExtensions().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        else if (!strcmp(argv[1], "getobjext"))
        {
            Tcl_SetResult(interp, (char *) self->GetExtension(atoi(argv[2])).c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        else if (!strcmp(argv[1], "postobjectmenu"))
        {
            Tcl_SetResult(interp, (char *) self->PostObjectMenu(atoi(argv[2])), TCL_VOLATILE);
            return TCL_OK;
        }
        
        else if (!strcmp(argv[1], "projection"))
            return self->ProjectionCommand(argc, argv);
        else if (!strcmp(argv[1], "animator"))
            return self->AnimatorCommand(argc, argv);
        else if (!strcmp(argv[1], "tellObject"))
        {
            Addressable *address = NULL;
            // PosableStateBase *objectPose = NULL;
            // if we have a valid option, do we have a valid name?
            if(argc > 2)
            {
                if (atoi(argv[2]))
                {
                    Object * o = mainWindow->getScene3D()->GetObjectPtr(atoi(argv[2]));
                    if(o)
                        return o->perform(argc - 3, &((const char **)argv)[3], interp);
                }
                if (!address)
                {
                    retval = self->findAddressIn(argv[2], ObjectNeighborhood, address, PosableNeighborhood);
                    if (retval != TCL_OK)
                        return retval;
                }
            }
            else
            {
                address = NULL;
            }
            
            if (!address)
            {
                Tcl_AppendResult(interp, "Invalid object: ", argv[2], NULL);
                return TCL_ERROR;
            }
            return address->perform(argc - 3, &((const char **)argv)[3], interp);
        }
        else if (!strcmp(argv[1], "tellObjects"))
        {
            if (argc >= 4)
            {
                DottedAddress *startingAddress = ObjectNeighborhood;
                std::vector<Addressable *> *items = NULL;
                int retval;
                std::string result, fullResult;

                if (argv[2][0] == '.')
                    startingAddress = rootAddress;

                items = startingAddress->findItems(argv[2], true);
                if (!items)
                {
                    Tcl_AppendResult(interp, "No object found that match the pattern ", argv[2], NULL);
                    return TCL_ERROR;
                }
                for (vector<Addressable *>::iterator i = items->begin(); i != items->end(); ++i)
                {
                    retval = (*i)->perform(argc - 3, &((const char **)argv)[3], interp);
                    if (retval != TCL_OK) //bail out on first failure
                        return retval;
                    result = Tcl_GetStringResult(interp);
                    if (result.find(" ") == std::string::npos)
                    {
                        if (fullResult.size() > 0)
                            fullResult += " ";
                        if (result.size())
                            fullResult += result;
                        else
                            fullResult += "{}";
                    }
                    else
                    {
                        fullResult += " {";
                        fullResult += result;
                        fullResult += "}";
                    }
                }
                Tcl_SetResult(interp, (char *) fullResult.c_str(), TCL_VOLATILE);
                return TCL_OK;
            }
            else
            {
                Tcl_AppendResult(interp, "usage: gz tellObjects <address-pattern> <command> [<argument>]*");
                return TCL_ERROR;
            }
        }
        else if (!strcmp(argv[1], "tellSelectedObject"))
            return Object::tellSelectedObject(argc - 2, &((const char **)argv)[2], interp);
        else if (!strcmp(argv[1], "address"))
            return self->AddressCommand(argc, argv);
        else if (!strcmp(argv[1], "event"))
            return self->EventCommand(argc, argv);
        else if (!strcmp(argv[1], "widgets"))
            return self->Widgets(argc,argv);
        else if (!strcmp(argv[1], "setTransparentBlendThreshold"))
        {
            if (argc > 2)
            {
                val = atof(argv[2]);
                if (val >= 0.0 && val <= 1.0)
                {
                    TextureMap::setTransparentBlendThreshold(atof(argv[2]));
                    return TCL_OK;
                }
            }
            Tcl_AppendResult(interp, "Argument expected between 0.0 and 1.0", NULL);
            return TCL_ERROR;
        }
        else if (!strcmp(argv[1], "time"))
            return self->TimeCommand(argc, argv);

        else if (!strcmp(argv[1], "htmlInfo"))
            return self->htmlInfo(argc,argv);


        else if (!strcmp(argv[1], "maxTransparencyLevel"))
            return self->MaxTransparencyLevelCommand(argc, argv);
        else if (!strcmp(argv[1], "setGlobalWindowContext"))
        {
            Addressable *address = NULL;
            Window3D *win = NULL;

            //if we have a valid option, do we have a valid name?
            if(argc > 2)
            {
                retval = self->findWindow(argv[2], address, win);
                if (retval != TCL_OK)
                    return retval;
                self->getScene3D()->setGlobalWindowContext(win);
            }
            else
            {
                self->getScene3D()->setGlobalWindowContext(NULL);
                return TCL_OK;
            }
            return TCL_OK;
        }
        else if (!strcmp(argv[1], "gutmBaseColor"))
        {
            char retval[16];

            if (argc >= 3)
                Gutm::surfaceColor[0] = (float)atof(argv[2]);
            if (argc >= 4)
                Gutm::surfaceColor[1] = (float)atof(argv[3]);
            if (argc >= 5)
                Gutm::surfaceColor[2] = (float)atof(argv[4]);
            if (argc >= 6)
                Gutm::surfaceColor[3] = (float)atof(argv[5]);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            sprintf(retval, "%f %f %f %f", Gutm::surfaceColor[0], Gutm::surfaceColor[1], Gutm::surfaceColor[2], Gutm::surfaceColor[3]);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
        else if (!strcmp(argv[1], "clutEditorFont"))
        {
            char retval[16];

            if (argc > 2)
                ClutEditor::displayFont = (void *) atoi(argv[2]);

            sprintf(retval, "%li", (long)ClutEditor::displayFont);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);

            return TCL_OK;
        }
        else if (!strcmp(argv[1], "defaultTides"))
        {
            DynamicTides *dTides = NULL;
            if (argc > 2)
            {
                Addressable *address;
                Object *ob;
                int ret;

                if (!argv[2] || argv[2][0] == '\0' || (argv[2][0] == '{' && argv[2][1] == '}'))
                    self->setDefaultTides(NULL);
                else
                {
                    ret = gzCommandInterface::findObject(argv[2], address, ob);
                    if (ret != TCL_OK)
                        return ret;
                    DYNAMIC_CAST(dTides, DynamicTides *, ob);
                    if (!dTides)
                    {
                        Tcl_AppendResult(interp, argv[2], " is not of type DynamicTides.  Discarding.", NULL);
                        return TCL_ERROR;
                    }
                    self->setDefaultTides(dTides);
                }
            }
            else
                dTides = self->getDefaultTides();
            if (dTides)
                Tcl_SetResult(interp, (char *)(dTides->getAddress()->getAddress().c_str()), TCL_VOLATILE);
            else
                Tcl_SetResult(interp, const_cast<char*>("{}"), TCL_VOLATILE);

            return TCL_OK;
        }
        else if (!strcmp(argv[1], "exit")) //Make sure we exit from the right thread
        {
            Window3D::exitNextIdle();
            return TCL_OK;
        }
#ifdef WIN32
        else if (!strcmp(argv[1], "sendkey"))
        {
            int arg, key;
            std::vector<int> keys;

            if (argc <= 2)
            {
                Tcl_AppendResult(interp, "usage: sendkey [Release:] <key>* [:Hold]", NULL);
                return TCL_ERROR;
            }
            for (arg = 2; arg < argc; arg++)
            {
                if (keyValues.count(argv[arg]) <= 0)
                {
                    Tcl_AppendResult(interp, "Unknown key for sendkey: ", argv[arg], ".\n",
                        NULL);
                    return TCL_ERROR;
                }
                keys.push_back(keyValues[argv[arg]]);
            }
            vector<int>::iterator i;
            //press keys in order
            for (i = keys.begin(); i != keys.end(); ++i)
            {
                if (*i == HOLD_KEYS) //don't process any more
                    return TCL_OK;
                if (*i == RELEASE_KEYS)
                    break; //get out of the key press loop
                if (*i >= 0)
                    keybd_event(*i , 0x41, KEYEVENTF_EXTENDEDKEY,  0 ); 
            }
            i--;
            //release keys in reverse order
            do {
                keybd_event( *i, 0x41, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0 );
            } while (i-- != keys.begin());
            return TCL_OK;
        }
        else if (!strcmp(argv[1], "saveClipboard"))
        {
#ifdef HAVE_FREEIMAGE
            fipWinImage image;

            if (argc <= 2)
            {
                Tcl_AppendResult(interp, "usage: saveClipboard <fullfilepath>", NULL);
                return TCL_ERROR;
            }
            if (!image.pasteFromClipboard())
            {
                Tcl_AppendResult(interp, "saveClipboard unable to paste image", NULL);
                return TCL_ERROR;
            }
            //image.convertToType(FIT_RGB16);
            image.save(argv[2]);
            return TCL_OK;
#else
            Tcl_AppendResult(interp, "saveClipboard command disabled", NULL);
            return TCL_ERROR;
#endif
        }
#endif
        Tcl_AppendResult(interp, "gz: Unknown option: ", argv[1], ".\n",
                        NULL);
    }
    Tcl_AppendResult(interp, "valid options are as follows:\n",
                        "  posable       "
                        "  animator      "
                        "  address       "
                        "  htmlInfo      "
                        "\n"
                        "  newobject     "
                        "  loadobject    "
                        "  saveobject    "
                        "  saveEdited    "
                        "\n"
                        "  hideobject    "
                        "  showobject    "
                        "  deleteobject  "
                        "  reloadobject  "
                        "\n"
                        "  tellObject    "
                        "  tellSelectedObject            "
                        "  explainobject "
                        "\n"
                        "  tellObjects   "
                        "  buildscene    "
                        "  clearScene    "
                        "  getobjext     "
                        "\n"
                        "  projection    "
                        "  time          "
//                       "\tstopwatch     "
                        "  event         "
                        "  widgets       "
                        "\n"
                        "  defaultTides  "
                        "  getObjectTypes"
                        "  getObjectExtensions           "
                        "\n"
                        "  setTransparentBlendThreshold  "
                        "  maxTransparencyLevel          "
                        "\n"
                        "  setGlobalWindowContext        "
                        "  gutmBaseColor "
                        "  clutEditorFont"
                        "\n"
                        "  ping          "
                        "  exit          "
#ifdef EYE_TRACKER
                        "  eyetrack      "
#endif
                        "\n",
                        NULL);
    return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes a Posable to be moved/rotated/scaled/etc.
*
*   argv[2]     option (set, get, offset, unset, list, etc.)
*   argv[3]     Dotted address of target posable to be altered
*   argv[4]     value-descriptor string (white space allowed)
*                           x - X position
*                           y - Y position
*                           z - Z position
*                           h - Heading
*                           p - Pitch
*                           r - Roll
*                           s - Scale
*                           S - 1/Scale (rja 12/11/2005 - to help prevent overflow in scripts)
*                       an optional class character is also allowed:
*                           C - Current value (default, if no class char is present)
*                           m - Minimum (lower limit on current value)
*                           M - Maximum (upper limit on current value)
*                           P - Preferred value (not implemented)
*                           H - "Home" value
*   argv[5] ... values (for set, offset options) as floats
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::PosableCommand(int argc,const char *argv[])
{
    //LogState ls(&gzlog,"gzCommandInterface::PosableCommand",TRACE);
    
    enum {InvalidCommand, SetCommand, GetCommand, OffsetCommand, UnsetCommand, 
        ListCommand, SubstituteCommand, CreateCommand, DeleteCommand,
        GetTypeCommand, CoupleCommand, DecoupleCommand};
    int command = InvalidCommand, currentArg;
    GLboolean badSyntax = GL_FALSE, missingValue = GL_FALSE;
    PosableStateBase *targetPose = NULL, *newPose = NULL;
    PosableStateInfo info;
    long changeMask = ChangeCurrentMask, typeMask= ChangeCurrentMask, individualMask;
    double value = 0.0;
    string descString;
    const char *currentChar;
    Addressable *address;
    DottedAddress *parent;
    RelationshipQueryResult *locks;
    PosableStateLock *lock;
    
    Tcl_SetResult(interp, const_cast<char*>(""), TCL_VOLATILE);
    
    //which option?
    if (argc < 5)
    {
        if ((argc >= 3) && (strcmp(argv[2], "list") == 0))
            command = ListCommand;
        else if ((argc >= 4) && (strcmp(argv[2], "type") == 0))
            command = GetTypeCommand;
        else if ((argc >= 4) && (strcmp(argv[2], "delete") == 0))
            command = DeleteCommand;
        else
            badSyntax = GL_TRUE;
    }
    else if (strcmp(argv[2], "set") == 0)
        command = SetCommand;
    else if (strcmp(argv[2], "get") == 0)
        command = GetCommand;
    else if (strcmp(argv[2], "offset") == 0)
        command = OffsetCommand;
    else if (strcmp(argv[2], "unset") == 0)
        command = UnsetCommand;
    else if (strcmp(argv[2], "substitute") == 0)
        command = SubstituteCommand;
    else if (strcmp(argv[2], "create") == 0)
        command = CreateCommand;
    else if (strcmp(argv[2], "delete") == 0)
        command = DeleteCommand;
    else if (strcmp(argv[2], "couple") == 0)
    {
        if (argc < 7)
            badSyntax = GL_TRUE;
        else
            command = CoupleCommand;
    }
    else if (strcmp(argv[2], "decouple") == 0)
    {
        if (argc < 6)
            badSyntax = GL_TRUE;
        else
            command = DecoupleCommand;
    }

    if (command == InvalidCommand)
        badSyntax = GL_TRUE;
    else if (argc >= 4)
    {
        if (findPosable(argv[3], address, targetPose, (command != ListCommand && command != CreateCommand))
            != TCL_OK)
            return TCL_ERROR;

        if (argc > 4 && command != ListCommand)  //get the descriptor string
            descString = argv[4];
    }

    //Process the description string for validity
    if ((argc > 4) && command < ListCommand)
    {
        currentArg = 5;
        for (currentChar = descString.data(); *currentChar; currentChar++)
        {
            if (command == SetCommand || command == OffsetCommand)
            {
                if (currentArg < argc)
                    value = atof(argv[currentArg]);
                else
                    missingValue = GL_TRUE;
            }
            
            
            switch (*currentChar)
            {
                case 'C':
                    changeMask &= ~ChangeTypeMask;
                    changeMask |= ChangeCurrentMask;
                    typeMask = ChangeCurrentMask;
                    break;
                case 'm':
                    changeMask &= ~ChangeTypeMask;
                    changeMask |= ChangeMinMask;
                    typeMask = ChangeMinMask;
                    break;
                case 'M':
                    changeMask &= ~ChangeTypeMask;
                    changeMask |= ChangeMaxMask;
                    typeMask = ChangeMaxMask;
                    break;
                case 'P':
                    changeMask &= ~ChangeTypeMask;
                    changeMask |= ChangePreferredMask;
                    typeMask = ChangePreferredMask;
                    break;
                case 'H':
                    changeMask &= ~ChangeTypeMask;
                    changeMask |= ChangeHomeMask;
                    typeMask = ChangeHomeMask;
                    break;
                case 'x':
                    info.position[X] = value;
                    individualMask = ChangeTranslateXMask;
                    goto bumpArg;
bumpArg:
                    if (targetPose->supportMask(typeMask | individualMask) & individualMask)
                        changeMask |= individualMask;
/*                    else //REMOVED by mdp 2/6/2008, I don't think we were doing anything with this anyway!
                        *currentChar = '.'; //indicate no support.
*/
                    if (missingValue)
                        badSyntax = GL_TRUE;
                    currentArg++;
                    break;
                case 'y':
                    info.position[Y] = value;
                    individualMask = ChangeTranslateYMask;
                    goto bumpArg;
                case 'z':
                    info.position[Z] = value;
                    individualMask = ChangeTranslateZMask;
                    goto bumpArg;
                case 'h':
                    info.orientation.axis[Yaw] = value;
                    individualMask = ChangeRotateYawMask;
                    goto bumpArg;
                case 'p':
                    info.orientation.axis[Pitch] = value;
                    individualMask = ChangeRotatePitchMask;
                    goto bumpArg;
                case 'r':
                    info.orientation.axis[Roll] = value;
                    individualMask = ChangeRotateRollMask;
                    goto bumpArg;
                case 's':
                    info.scale = value;
                    individualMask = ChangeScaleMask;
                    goto bumpArg;
                case 'S':
                    info.scale = 1.0/value;
                    individualMask = ChangeScaleMask;
                    goto bumpArg;
                case ' ': case '\t': case '\n':
                    break;
                default:
                    Tcl_AppendResult(interp,"Bad value descriptor string \"", argv[4], "\".  Must consist of the following:\n",
                                            "    x - X position\n",
                                            "    y - Y position\n",
                                            "    z - Z position\n",
                                            "    h - Heading\n",
                                            "    p - Pitch\n",
                                            "    r - Roll\n",
                                            "    s - Scale\n",
                                            "    S - 1/Scale\n",
                                            "an optional class character is also allowed:\n",
                                            "    C - Current value (default, if no class char is present)\n",
                                            "    m - Minimum (lower limit on current value)\n",
                                            "    M - Maximum (upper limit on current value)\n",
                                            "    P - Preferred value (not implemented)\n",
                                            "    H - \"Home\" value\n",
                                    NULL);
                    return TCL_ERROR;
            }
        }
    }

    //report any bad syntax (and exit)
    if (badSyntax)
    {
        Tcl_AppendResult(interp, "Incorrect use of posable subcommand.  Usage options:\n",
                                "gz posable get <name> <value-descriptor>\n",
                                "gz posable unset <name> <value-descriptor>\n",
                                "gz posable set <name> <value-descriptor> [<value>...]\n",
                                "gz posable offset <name> <value-descriptor> [<value>...]\n",
                                "gz posable list [<name>]\n",
                                "gz posable substitute <name> <posable-type>\n",
                                "gz posable create <parent> <newname> <posable-type>\n",
                                "gz posable delete <name>\n",
                                "gz posable type <name>\n",
                                "gz posable couple <name> to <source-name> <coupling-type>\n",
                                "gz posable decouple <name> and <source-name> [<coupling-type>]\n",
                        NULL);
        return TCL_ERROR;
    }

    //perform the actual command.
    stringstream returnString;
    returnString << fixed << setprecision(12);
    string name;
    switch (command)
    {
        case SetCommand:
            targetPose->setPoseInfo(info, changeMask);
            break;
        case OffsetCommand:
            targetPose->offsetPoseInfoBy(info, changeMask);
            break;
        case UnsetCommand:
            targetPose->unsetPoseInfo(changeMask);
            break;
        case GetCommand:
            info = targetPose->getPoseInfo(changeMask);
            returnString.str("");
            for (currentChar = descString.data(); *currentChar; currentChar++)
            {
                switch (*currentChar)
                {
                    case 'C': case 'm': case 'M': case 'P': case 'H':
                    case ' ': case '\t': case '\n':
                        continue;
                        break;
                    case 'x':
                        value = info.position[X];
                        break;
                    case 'y':
                        value = info.position[Y];
                        break;
                    case 'z':
                        value = info.position[Z];
                        break;
                    case 'h':
                        value = info.orientation.axis[Yaw];
                        break;
                    case 'p':
                        value = info.orientation.axis[Pitch];
                        break;
                    case 'r':
                        value = info.orientation.axis[Roll];
                        break;
                    case 's':
                        value = info.scale;
                        break;
                    case 'S':
                        value = 1.0/info.scale;
                        break;
                }
                if (returnString.str().empty())
                {
                    if (*currentChar == '.')
                        returnString << "{}";
                    else
                        returnString << value;
                }
                else
                {
                    if (*currentChar == '.')
                        returnString << " {}";
                    else
                        returnString << " " << value;
                }
            }
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            break;
        case ListCommand:
            if (argc >= 4)
                parent = address->getAddress();
            else
                parent = PosableNeighborhood;
            listChildrenOf(parent);
            break;
        case CreateCommand:
            if (argc >= 6)
                descString = argv[5];
            else
                descString = "object";
            if (descString == "object")
                newPose = new ObjectPoseState();
            else if (descString == "translate")
                newPose = new TranslatableState();
            else if (descString == "translate_wrapper")
                newPose = (PosableStateWrapper *)new TranslatableWrapper();
            else if (descString == "exaggerate")
                newPose = new ExaggeratableState();
            else if (descString == "window")
                newPose = new WindowPoseState(NULL);
            else
            {
                Tcl_AppendResult(interp, "Invalid <posable-type>: ", descString.c_str(), ".  Should be one of\n",
                                        "object\n",
                                        "translate\n",
                                        "translate_wrapper\n",
                                        "exaggerate\n",
                                        "window\n",
                                NULL);
                return TCL_ERROR;
            }
            newPose->publishName(argv[4]);
            address->getAddress()->addChild(newPose->getAddress());
            break;
        case DeleteCommand:
            if (targetPose->getUser())
            {
                Tcl_AppendResult(interp, "Cannot delete ", argv[3], " because it is in use.\n",
                                "Either destroy its user or substitute it to delete it.",
                                NULL);
                return TCL_ERROR;
            }
            delete targetPose;
            break;
        case SubstituteCommand:
            if (descString == "object")
                newPose = new ObjectPoseState(targetPose);
            else if (descString == "translate")
                newPose = new TranslatableState(targetPose);
            else if (descString == "exaggerate")
                newPose = new ExaggeratableState(targetPose);
            else
            {
                Tcl_AppendResult(interp, "Invalid <posable-type>: ", descString.c_str(), ".  Should be one of\n",
                                        "object\n",
                                        "translate\n",
                                        "exaggerate\n",
                                NULL);
                return TCL_ERROR;
            }
            break;
        case GetTypeCommand:
            name = typeid(*targetPose).name();
            name = name.substr(6); //skip past "class "
            switch (name[0]) 
            {
                case 'W': //WindowPoseState
                    returnString << "window";
                    break;
                case 'O': //ObjectPoseState
                    returnString << "object";
                    break;
                case 'T': //TranslatableState
                    if (name == "TranslatableState")
                        returnString << "translate";
                    else if (name == "TranslatableWrapper")
                        returnString << "translate_wrapper";
                    break;
                case 'E': //ExaggeratableState
                    returnString << "exaggerate";
                    break;
                default:
                    returnString << "unknown";
                    break;
            }
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            break;
        case CoupleCommand:
            if (findPosable(argv[5], address, newPose) != TCL_OK)
                return TCL_ERROR;
            changeMask = parseCouplingType(argv[6]);
            if (changeMask < 0)
                return TCL_ERROR;
            locks = relationshipManager->getLocksBetween(newPose, targetPose);
            if (locks->numResults() == 0)
            {
                lock = new PosableStateLock(newPose->getUser(), newPose,
                                            targetPose, changeMask);
                returnString << "Coupling_" << getUniqueCount();
                lock->publishName(returnString.str());
                CouplingNeighborhood->addChild(lock->getAddress());
            }
            else //assume first one is good enough for us.
            {
                lock = (PosableStateLock*) locks->first();
                lock->lock(changeMask);
            }
            delete locks;
            if (lock->getAddress())
                Tcl_SetResult(interp, (char*)(lock->getAddress()->getAddress().c_str()), TCL_VOLATILE);
            break;
        case DecoupleCommand:
            if (findPosable(argv[5], address, newPose) != TCL_OK)
                return TCL_ERROR;
            if (argc >= 7)
                changeMask = parseCouplingType(argv[6]);
            else
                changeMask = ChangeAll;
            if (changeMask < 0)
                return TCL_ERROR;
            locks = relationshipManager->getLocksBetween(newPose, targetPose);
            if (locks->numResults() == 0)
            {
                Tcl_AppendResult(interp, "No coupling exists with ", argv[5], ".\n",
                                NULL);
                return TCL_ERROR;
            }
            else //assume first one is good enough for us.
            {
                lock = (PosableStateLock*) locks->first();
                lock->unlock(changeMask);
            }
            delete locks;
            if (lock->getAddress())
                Tcl_SetResult(interp, (char*)lock->getAddress()->getAddress().c_str(), TCL_VOLATILE);
            break;
        default:
            break;
    }

    return TCL_OK;
}

#ifdef EYE_TRACKER
/*---------------------------------------------------------------------------*/
int gzCommandInterface::EyeTrackCommand(int argc, const char *argv[])
{
    int numGP;
    int x, y, time;
        
    if (argc >= 3)
    {
        if (!strcmp(argv[2], "version"))
        {
            stringstream returnString;
            returnString << GetSDKVersion();
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(argv[2], "init"))
        {
            //DisableMouseInput();
            stringstream returnString;
            returnString << GetQGOnFlag() << " " << GetESEnabledFlag();
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(argv[2], "exit"))
        {
            ExitQuickGlance();
            stringstream returnString;
            returnString << GetQGOnFlag() << " " << GetESEnabledFlag();
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(argv[2], "start"))
        {
            StartGPCollection(SCStandard, 0);
            stringstream returnString;
            returnString << GetQGOnFlag() << " " << GetESEnabledFlag();
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(argv[2], "stop"))
        {
            StopGPCollection();
            return TCL_OK;
        }
        if (!strcmp(argv[2], "poll"))
        {
            numGP = GetNumGP();
            if (numGP <= 0)
                Tcl_SetResult(interp, "{}", TCL_VOLATILE);
            else
            {
                ReadGP(x, y, time);
                stringstream returnString;
                returnString << x << " " << y << " " << time;
                Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            }
            return TCL_OK;
        }
        if (!strcmp(argv[2], "calibrate"))
        {
            if (argc >= 4)
            {
                if (!strcmp(argv[3], "quit"))
                {
                    QuitCalUpdate();
                    return TCL_OK;
                }
                else if (!strcmp(argv[3], "save"))
                {
                    SaveAndQuitCalUpdate();
                    return TCL_OK;
                }
            }
            else
            {
                CallCalUpdate();
                return TCL_OK;
            }
        }       
        if (!strcmp(argv[2], "cursor"))
        {
            if (argc >= 4)
            {
                if (!strcmp(argv[3], "on"))
                {
                    SetEnableEyeControl();
                    return TCL_OK;
                }
                else if (!strcmp(argv[3], "off"))
                {
                    ResetEnableEyeControl();
                    return TCL_OK;
                }
            }
        }
    }

    Tcl_AppendResult(interp, "Incorrect use of eyetrack subcommand.  Usage options:\n",
                            "gz eyetrack version\n",
                            "gz eyetrack init\n",
                            "gz eyetrack start\n",
                            "gz eyetrack stop\n",
                            "gz eyetrack poll\n",
                            "gz eyetrack exit\n",
                            "gz eyetrack cursor {on | off}\n",
                            "gz eyetrack calibrate [quit | save]\n",
                        NULL);
    return TCL_ERROR;
}
#endif

/*---------------------------------------------------------------------------*/
/**
*   This function controls the stopwatch with simple functions like stop and
*   start.
*
*   @param  argv[2]     subcommand.
*   @return             TCL_OK / TCL_ERROR
*/
/*
int gzCommandInterface::ControlStopwatch(int argc, const char *argv[])
{
    enum {InvalidCommand, StartCommand, StopCommand, ContinueCommand, ReadCommand, 
        SinceLastReadCommand};
    int command = InvalidCommand;

    //which option?
    if (argc < 3)
        badSyntax = GL_TRUE;
    else if (strcmp(argv[2], "start") == 0)
        command = StartCommand;
    else if (strcmp(argv[2], "stop") == 0)
        command = StopCommand;
    else if (strcmp(argv[2], "continue") == 0)
        command = ContinueCommand;
    else if (strcmp(argv[2], "read") == 0)
        command = ReadCommand;
    else if (strcmp(argv[2], "sinceLastRead") == 0)
        command = SinceLastReadCommand;

    //report any bad syntax (and exit)
    if (badSyntax)
    {
        Tcl_AppendResult(interp, "Incorrect use of stopwatch subcommand.  Usage options:\n",
                                "gz stopwatch start\n",
                                "gz stopwatch stop\n",
                                "gz stopwatch continue\n",
                                "gz stopwatch read\n",
                                "gz stopwatch sinceLastRead [<count-as-read?>]\n",
                        NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}
*/
/*---------------------------------------------------------------------------*/
/**
*   This function causes an object to be loaded into the current Scene3D
*
*   argv[2]     Filename of object file to be loaded
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::LoadObject(int argc, const char *argv[])
{
    Object *object;

    object = scene3d->LoadObject(argv[2], interp,NULL,true);
    if (object)
        scene3d->AddObject(object, interp);
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function creates a new object to be inserted into the current Scene3D
*
*   argv[2]     Type or extention (ex: .gutm) of object to be created.
* argv[3]   Label to assign to the object
* argv[4]   x value, or, posSelfInit flag, meaning th object's constructor
*                   is responsible to assign it's position, so we should not override it
* argv[5,6] y and z values.
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::NewObject(int argc, const char *argv[])
{
    if(argc==4)
    {
        scene3d->NewObject((char *)argv[2], interp, (char *)argv[3]);
    }
    else if(argc == 5 && strcmp(argv[4],"posSelfInit")==0)
    {
        scene3d->NewObject((char *)argv[2], interp, (char *)argv[3],0.0,0.0,0.0,1);
    }
    else if(argc==7)
    {
        scene3d->NewObject((char *)argv[2], interp, (char *)argv[3], 
            atof(argv[4]),atof(argv[5]),atof(argv[6]));
    }
    else
    {
        Tcl_AppendResult(interp,"Usage: newobject type label [posSelfInit| x y z]\n  When position is not supplied, it will default to 0,0,0 unless posSelfInit flag is found, indicating the object will take care of initializing its position.", NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes an object in the current scene to be saved
*
*   argv[2]     Filename of object file to be loaded
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::SaveObject(int argc, const char *argv[])
{
    scene3d->SaveObject(atoi(argv[2]), (char *)argv[3]);
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes all edited objects in the current scene to be saved
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::SaveEditedObjects(int argc, const char *argv[])
{
    scene3d->SaveEditedObjects();
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes an object to be skipped during subsequent rendering
*
*   argv[2]     Index of object to be hidden
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::HideObject(int argc, const char *argv[])
{
    scene3d->SetInvisible(atoi(argv[2]));
    return TCL_OK;
}


/*---------------------------------------------------------------------------*/
/**
*   This function causes an object to be marked for deletion
*
*   argv[2]     Index of object to be deleted
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::DeleteObject(int argc, const char *argv[])
{
    scene3d->GetObjectPtr(atoi(argv[2]))->Unload();
    return TCL_OK;
}

/**
Queries or sets status of widgets
*/
int gzCommandInterface::Widgets(int argc, const char *argv[])
{
    stringstream returnString;
    switch(argc)
    {
    case 3:
        scene3d->SetWidgetsVisible(isTrue(argv[2]));
    case 2:
        returnString << scene3d->AreWidgetsVisible() << endl;
        Tcl_SetResult(interp,(char *)returnString.str().c_str(), TCL_VOLATILE);
        return TCL_OK;
    default:
        Tcl_SetResult(interp,const_cast<char*>("Usage: gz widgets [on/off]\n"), TCL_VOLATILE);
    }
    return TCL_ERROR;
}


// /*---------------------------------------------------------------------------*/
// /**
// *  This functions starts the mpeg recording process
// *    @return             TCL_OK / TCL_ERROR
// */
// int gzCommandInterface::StartVideoRecorder(int argc, const char *argv[])
// {
//     return mainWindow->StartVideoRecorder(argc,argv);
// }
// 
// /*---------------------------------------------------------------------------*/
// /**
// *  This functions records one frame of a movie
// *    @return             TCL_OK / TCL_ERROR
// */
// int gzCommandInterface::RecordFrame(int argc, const char *argv[])
// {
//     return mainWindow->RecordFrame(argc,argv);
// }
// 
// /*---------------------------------------------------------------------------*/
// /**
// *  This adds a frame to the accumulation frame
// *    @return             TCL_OK / TCL_ERROR
// */
// int gzCommandInterface::AddFrame(int argc, const char *argv[])
// {
//     return mainWindow->AddFrame(argc,argv);
// }
// 
// 
// /*---------------------------------------------------------------------------*/
// /**
// *  This functions stops the mpeg recording process
// *    @return             TCL_OK / TCL_ERROR
// */
// int gzCommandInterface::StopVideoRecorder(int argc, const char *argv[])
// {
//     return mainWindow->StopVideoRecorder(argc,argv);
// }

/*---------------------------------------------------------------------------*/
/**
*  Return html formatted information
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::htmlInfo(int argc, const char *argv[])
{

    if (argc > 2)
    {
        string c(argv[2]);
        if (c == "object")
            if (argc == 4)
            {
                int oid = atoi(argv[3]);
                Object *o = scene3d->GetObjectPtr(oid);
                stringstream returnString;
                if (o)
                {
                    o->getHtmlInfo(returnString,1);
                    Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
                    return TCL_OK;
                }
            }
    }
    return TCL_ERROR;
}


/*---------------------------------------------------------------------------*/
/**
*   This function causes an object to be reloaded.
*
*   argv[2]     Index of object to be reloaded
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::ReloadObject(int argc, const char *argv[])
{
    scene3d->ReloadObject(scene3d->GetObjectPtr(atoi(argv[2])));
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes an object to provide information about itself through
*   the GUI.
*
*   argv[2]     Index of object to be explained
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::ExplainObject(int argc, const char *argv[])
{
    scene3d->GetObjectPtr(atoi(argv[2]))->ShowInfo();
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes an object to be included during subsequent rendering
*
*   argv[2]     Index of object to be shown
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::ShowObject(int argc, const char *argv[])
{
    scene3d->SetVisible(atoi(argv[2]));
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function loads a ConversionGrid from the passed file and converts it
*   to a GUTM, which is loaded into the current Scene3D.
*
*   @param  argv[2]     Filename of grid file to be loaded
*   @return             TCL_OK / TCL_ERROR
int gzCommandInterface::Import(int argc, const char *argv[])
{
    ConversionGrid *grid;

    if (argc < 3)
    {
        Tcl_AppendResult(interp, "Incorrect use of import subcommand.  Usage options:\n",
                                "gz import <filename>\n",
                        NULL);
        return TCL_ERROR;
    }

    grid = new ConversionGrid;
    if (grid->SetFilename(argv[2]) == 0)
    {
        if (!grid->Load())
        {
            Tcl_AppendResult(interp, "Error loading grid.\n", NULL);
            return TCL_ERROR;
        }
    }
    //scene3d->LoadObject(argv[2], interp);

    return TCL_OK;
}
*/

/*---------------------------------------------------------------------------*/
/**
*   This function writes the filename for the object at the passed index into
*   $outfile (in Tcl world).
*
*   argv[2]     Index of object whose filename is to be saved.
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::BuildScene(int argc, const char *argv[])
{
//  char tclstr[200];

    scene3d->BuildScene(interp, argv[2]);
    return TCL_OK;
    
//    sprintf(tclstr,"puts $outfile textures");
//  mainwin->Tcl_Eval(tclstr);
//    scene3d->BuildTextureScene(interp, argv[2]);
//    sprintf(tclstr,"puts $outfile objects");
//  mainwin->Tcl_Eval(tclstr);
//    scene3d->BuildObjectScene(interp, argv[2]);
//  return TCL_OK;
}

int gzCommandInterface::SaveScene(int argc, const char *argv[])
{
    scene3d->SaveScene(interp, argv[2],argv[3]);
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function clears the associated Scene3D.
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::ClearScene()
{
    Object *obj;

    scene3d->ResetIterator();
    for (obj = scene3d->GetObjectPtr(); obj != NULL; obj = scene3d->Next())
        obj->Unload();
    gz4d::ogr::SpatialReference sr;
    scene3d->setSpatialReference(sr, true);

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function returns the object types registered with the associated
*   Scene3D.
*
*   @return             String: space-delimited list of object types
*/
string gzCommandInterface::GetObjectTypes()
{
    return scene3d->GetObjectTypes();
}

/*---------------------------------------------------------------------------*/
/**
*   This function returns the extensions registered with the associated
*   Scene3D.
*
*   @return             String: space-delimited list of extensions
*/
string gzCommandInterface::GetObjectExtensions()
{
    return scene3d->GetObjectExtensions();
}

/*---------------------------------------------------------------------------*/
/**
*   This function returns the extension registered with the associated
*   Scene3D that corresponds to the object at the passed index.
*
*   @return             String containing the extension
*/
string gzCommandInterface::GetExtension(int object)
{
    return scene3d->GetExtension(object);
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes a projection to be applied/inverted.
*
*   argv[2]     option (apply, invert)
*   argv[3]     name of projection
*   argv[4]...  arguments
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::ProjectionCommand(int argc, const char *argv[])
{
    enum {InvalidCommand, ApplyCommand, InvertCommand, LongToUTM, SetCurrent};
    int command = InvalidCommand;
    GLboolean badSyntax = GL_FALSE;
    double lat_or_north = 0.0, lon_or_east = 0.0;

    if (argc <= 2)
        badSyntax = GL_TRUE;
    else if (strcmp(argv[2], "apply") == 0)
        command = ApplyCommand;
    else if (strcmp(argv[2], "invert") == 0)
        command = InvertCommand;
    else if (strcmp(argv[2], "longToUTM") == 0)
        command = LongToUTM;
    else if (strcmp(argv[2], "setCurrent") == 0)
        command = SetCurrent;

    if (command == InvalidCommand)
        badSyntax = GL_TRUE;

    //get other arguments
    if (!badSyntax)
    {
        switch (command)
        {
            case ApplyCommand:
            case InvertCommand:
                if (argc != 6)
                    badSyntax = GL_TRUE;
                else
                {
                    lon_or_east = atof(argv[4]);
                    lat_or_north = atof(argv[5]);
                }
                break;
            case LongToUTM:
                if (argc != 4)
                    badSyntax = GL_TRUE;
                else
                    lon_or_east = atof(argv[3]);
                break;
            case SetCurrent:
                if (argc != 4)
                    badSyntax = GL_TRUE;
                break;
            default:
                badSyntax = GL_TRUE;
                break;
        }
    }

    //report any bad syntax (and exit)
    if (badSyntax)
    {
        Tcl_AppendResult(interp, "Incorrect use of projection subcommand.  Usage options:\n",
                                "gz projection apply <name> <longitude> <latitude>\n",
                                "gz projection invert <name> <easting> <northing>\n",
                                "gz projection longToUTM <longitude>\n",
                                "gz projection setCurrent <name>\n",
                        NULL);
        return TCL_ERROR;
    }

    //perform the actual command.
    stringstream returnString;
    returnString << fixed << setprecision(12);
    if (command == ApplyCommand)
    {
        gz4d::geo::Projection p(argv[3]);
        gz4d::Point<double> pos = p(lat_or_north,lon_or_east);
        returnString << std::fixed << pos[X] << " " << pos[Y];
        Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
    }
    else if (command == InvertCommand)
    {
        gz4d::geo::Projection p(argv[3]);
        gz4d::geo::Position<double> pos = p.inverse(lon_or_east, lat_or_north);
        returnString << std::fixed << pos.longitude.value() << " " << pos.latitude.value();
        Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
    }
    else if (command == LongToUTM)
    {
        returnString << "+proj=utm +ellps=WGS84 +zone=" << gz4d::geo::UTMZone(lon_or_east);
        Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
    }
    else if (command == SetCurrent)
    {
        gz4d::ogr::SpatialReference p;
        p.SetFromUserInput(argv[3]);
        scene3d->setSpatialReference(p, true);
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes an animator to be created/used/destroyed.
*
*   argv[2]     option (create, delete, list, types)
*   argv[3]     address of animator
*   argv[4]...  arguments
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::AnimatorCommand(int argc, const char *argv[])
{
    enum {InvalidCommand, CreateCommand, ListCommand, TypesCommand,
        DeleteCommand};
    int command = InvalidCommand, i, end, retval;
    GLboolean badSyntax = GL_FALSE;
    Addressable *address;
    DottedAddress *parent;
    Animator *animator = NULL, *newAnimator = NULL;
    Animator::RegistrationEntry **registry, *creationEntry = NULL;
    PosableStateBase *newPose;

    if (argc < 4)
    {
        if (argc >= 3)
        {
            if (strcmp(argv[2], "list") == 0)
                command = ListCommand;
            else if (strcmp(argv[2], "types") == 0)
                command = TypesCommand;
            else
                badSyntax = GL_TRUE;
        }
        else
            badSyntax = GL_TRUE;
    }
    else if (strcmp(argv[2], "create") == 0)
        command = CreateCommand;
    else if (strcmp(argv[2], "list") == 0)
        command = ListCommand;
    else if (strcmp(argv[2], "types") == 0)
        command = TypesCommand;
    else if (strcmp(argv[2], "delete") == 0)
        command = DeleteCommand;

    if (command == InvalidCommand)
        badSyntax = GL_TRUE;
    else if (argc >= 4)
    {
        //if we have a valid option, do we have a valid name?
        retval = findAddressIn(argv[3], AnimatorNeighborhood, address);
        if (retval != TCL_OK)
            return retval;
        if (!address || (command != ListCommand && command != CreateCommand))
        {
            if (address)
                DYNAMIC_CAST(animator, Animator*, address);
            if (!animator)
            {
                Tcl_AppendResult(interp, "Invalid animator: ", argv[3], NULL);
                return TCL_ERROR;
            }
        }
    }

    //get other arguments
    if (!badSyntax)
    {
        switch (command)
        {
            case CreateCommand:
                if (argc < 6)
                    badSyntax = GL_TRUE;
                else
                {
                    creationEntry = Animator::getRegistryEntry(argv[5]);
                    if (!creationEntry)
                    {
                        Tcl_AppendResult(interp, "Invalid animator type ", argv[5],
                                        ".  Use \"gz animator types\" to list valid types.\n", NULL);
                        return TCL_ERROR;
                    }
                }
                break;
            case DeleteCommand:
                if (argc != 4)
                    badSyntax = GL_TRUE;
                break;
        }
    }

    //report any bad syntax (and exit)
    if (badSyntax)
    {
        Tcl_AppendResult(interp, "Incorrect use of animator subcommand.  Usage options:\n",
                                "gz animator types\n",
                                "gz animator create <parent> <newname> <type> [<target> <operand>*]\n",
                                "gz animator delete <name>\n",
                                "gz animator list [<name>]\n",
                        NULL);
        return TCL_ERROR;
    }

    //perform the actual command.
    stringstream returnString;
    switch (command)
    {
        case CreateCommand:
            newAnimator = creationEntry->create();
            newAnimator->publishName(argv[4]);
            address->getAddress()->addChild(newAnimator->getAddress());
            if (argc >= 7)
            {
                if (findPosable(argv[6], address, newPose) != TCL_OK)
                    return TCL_ERROR;
                newAnimator->replaceResultant(newPose);
                for (i = 7; i < argc; i++)
                {
                    if (findPosable(argv[i], address, newPose) != TCL_OK)
                        return TCL_ERROR;
                    newAnimator->addOperand(newPose);
                }
            }
            globalInputMediator->PushInteractor(newAnimator);
            //joystickEventMediator->PushInteractor(newAnimator);
            //% gz animator create .Animator test ImitatingAnimator Window.main Object.r509_gutm
            break;
        case DeleteCommand:
            delete animator;
            break;
        case ListCommand:
            if (argc >= 4)
                parent = address->getAddress();
            else
                parent = AnimatorNeighborhood;
            listChildrenOf(parent);
            break;
        case TypesCommand:
            registry = Animator::getRegistry();
            end = Animator::getRegistrySize();
            for (i = 0; i < end; i++)
            {
                if (i != 0)
                    returnString << " ";
                returnString << registry[i]->className;
            }
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            break;
        default:
            break;
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function causes an address to be created/moved/destroyed.
*
*   argv[2]     option (create, delete, exists, list, type, reparent, numchilden, addchild)
*   argv[3]...  arguments
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::AddressCommand(int argc, const char *argv[])
{
    
    enum {InvalidCommand, CreateCommand, ListCommand, TypeCommand,
        DeleteCommand, ExistsCommand, ReparentCommand, RemoveChildCommand,
        NumChildrenCommand, AddChildCommand, htmlInfoCommand};
    int command = InvalidCommand;
    GLboolean badSyntax = GL_FALSE, minList = GL_FALSE;
    Addressable *addressable = NULL;
    DottedAddress *targetAddress = NULL, *otherAddress = NULL;

    if (argc < 4)
    {
        if (argc >= 3 && (strcmp(argv[2], "list") == 0 || strcmp(argv[2], "minlist") == 0))
        {
            command = ListCommand;
            if (argv[2][0] == 'm')
                minList = GL_TRUE;
        }
        else
            badSyntax = GL_TRUE;
    }
    else if (strcmp(argv[2], "create") == 0 && argc >= 5)
        command = CreateCommand;
    else if (strcmp(argv[2], "list") == 0 || strcmp(argv[2], "minlist") == 0)
    {
        command = ListCommand;
        if (argv[2][0] == 'm')
            minList = GL_TRUE;
    }
    else if (strcmp(argv[2], "type") == 0)
        command = TypeCommand;
    else if (strcmp(argv[2], "delete") == 0)
        command = DeleteCommand;
    else if (strcmp(argv[2], "exists") == 0)
        command = ExistsCommand;
    else if (strcmp(argv[2], "addchild") == 0)
        command = AddChildCommand;
    else if (strcmp(argv[2], "reparent") == 0)
        command = ReparentCommand;
    else if (strcmp(argv[2], "removechild") == 0)
        command = RemoveChildCommand;
    else if (strcmp(argv[2], "numchildren") == 0)
        command = NumChildrenCommand;
    else if (strcmp(argv[2], "htmlInfo") == 0)
        command = htmlInfoCommand;

    if (command == InvalidCommand)
        badSyntax = GL_TRUE;
    else if (argc >= 4)
    {
        //if we have a valid option, do we have a valid name?
        if ((command != DeleteCommand || command != ReparentCommand || command != ExistsCommand)
            && strcmp(argv[3], "toplevel") == 0)
            targetAddress = rootAddress;
        else
            targetAddress = rootAddress->find(argv[3]);

        if (command == ExistsCommand)
        {
            stringstream returnString;
            returnString << (targetAddress != NULL);
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            return TCL_OK;
        }
        if (!targetAddress)
        {
            Tcl_AppendResult(interp, "Invalid address: ", argv[3], NULL);
            return TCL_ERROR;
        }

        if (argc >= 5 && command != CreateCommand)
        {
            if ((command == ReparentCommand) && strcmp(argv[4], "toplevel") == 0)
                otherAddress = rootAddress;
            else
                otherAddress = rootAddress->find(argv[4]);
            if (!otherAddress)
            {
                Tcl_AppendResult(interp, "Invalid address: ", argv[4], NULL);
                return TCL_ERROR;
            }
        }
    }

    //report any bad syntax (and exit)
    if (badSyntax)
    {
        Tcl_AppendResult(interp, "Incorrect use of address subcommand.  Usage options:\n",
                                "gz address create <parent> <newname>\n",
                                "gz address delete <name>\n",
                                "gz address exists <name>\n",
                                "gz address list [<name>]\n",
                                "gz address minlist [<name>]\n",
                                "gz address numchildren <name>\n",
                                "gz address type <name>\n",
                                "gz address addchild <parent-name> <child-name>\n",
                                "gz address removechild <parent-name> <child-name>\n",
                                "gz address reparent <child-name> <parent-name>\n",
                                "gz address htmlInfo\n",
                        NULL);
        return TCL_ERROR;
    }

    //perform the actual command.
    if (command != CreateCommand && command != ListCommand)
        addressable = targetAddress->getItem();

    stringstream returnString;
    switch (command)
    {
        case CreateCommand:
            if (strchr(argv[4], '.'))
            {
                Tcl_AppendResult(interp, "Invalid name ", argv[4],
                                ".  Name cannot contain dots.", NULL);
                return TCL_ERROR;
            }
            addressable = new Addressable(argv[4]);
            targetAddress->addChild(addressable->getAddress());
            break;
        case DeleteCommand:
            if (targetAddress->getNumChildren() == 0)
                delete addressable;
            else
            {
                Tcl_AppendResult(interp, "Cannot delete ", argv[3],
                                ".  Delete its children first.", NULL);
                return TCL_ERROR;
            }
            break;
        case ListCommand:
            if (!targetAddress)
                targetAddress = rootAddress;
            listChildrenOf(targetAddress, !minList);
            break;
        case TypeCommand:
            Tcl_SetResult(interp, (char *)&(typeid(*addressable).name()[6]), TCL_VOLATILE);
            break;
        case AddChildCommand:
            targetAddress->addChild(otherAddress);
            break;
        case RemoveChildCommand:
            targetAddress->removeChild(otherAddress);
            break;
        case ReparentCommand:
            targetAddress->reparent(otherAddress);
            break;
        case NumChildrenCommand:
            returnString << targetAddress->getNumChildren();
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            break;
        case htmlInfoCommand:
        {
            returnString.str("");
            targetAddress->getItem()->getHtmlInfo(returnString,1);
            Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
            break;
        }
        default:
            break;
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function queries the event system and allows handlers to be
*   created/destroyed.
*
*   argv[2]     option (handler)
*   argv[3]...  arguments
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::EventCommand(int argc, const char *argv[])
{
    enum {InvalidCommand, CreateCommand, DeleteCommand, TriggerCommand,
        SinkCommand, HandlerCommand, MediatorCommand, TentacleCommand};
    int command = InvalidCommand, subcommand = InvalidCommand, scriptArg, retval;
    GLboolean badSyntax = GL_FALSE;
    DottedAddress *adr = NULL;
    Addressable *address = NULL;
    EventHandler *handler = NULL;
    EventType *event = NULL;
    InputMediator *mediator = NULL;
    InputTentacle *tentacle = NULL;
    GLboolean createSink = GL_FALSE;
    unsigned int eventType = Interactor::InvalidInput;

    if (argc < 4)
    {
        badSyntax = GL_TRUE;
    }
    else if (strcmp(argv[2], "create") == 0)
        command = CreateCommand;
    else if (strcmp(argv[2], "delete") == 0)
        command = DeleteCommand;
    else if (strcmp(argv[2], "mediator") == 0)
        command = MediatorCommand;
    else if (strcmp(argv[2], "sink") == 0)
    {
        command = HandlerCommand;
        createSink = GL_TRUE;
    }
    else if (strcmp(argv[2], "handler") == 0)
        command = HandlerCommand;
    else if (strcmp(argv[2], "tentacle") == 0)
        command = TentacleCommand;
    if (command == MediatorCommand || command == HandlerCommand || command == TentacleCommand)
    {
        if (strcmp(argv[3], "create") == 0 && (((command == MediatorCommand || command == TentacleCommand) && argc >= 5) || argc >= 6))
            subcommand = CreateCommand;
        else if (strcmp(argv[3], "delete") == 0 && argc >= 5)
            subcommand = DeleteCommand;
        else
            badSyntax = GL_TRUE;
    }

    //report any bad syntax (and exit)
    if (badSyntax)
    {
        Tcl_AppendResult(interp, "Incorrect use of event subcommand.  Usage options:\n",
                                "gz event {create | delete} <name>\n",
                                "gz event handler create <name> [for <event>] <script>\n",
                                "gz event handler delete <name>\n",
                                "gz event mediator {create | delete} <name>\n",
                                "gz event sink create <name> for <event>\n",
                                "gz event tentacle {create | delete} <name>\n",
                        NULL);
        return TCL_ERROR;
    }

    //perform the command
    switch (command)
    {
        case HandlerCommand:
            switch (subcommand)
            {
                case CreateCommand:
                    scriptArg = 5;
                    if (!strcmp(argv[5], "for"))
                    {
                        if (argc > 6)
                            eventType = convertEvent(argv[6], mainWindow);
                        else
                        {
                            Tcl_AppendResult(interp, "Missing event and script after keyword 'for'", NULL);
                            return TCL_ERROR;
                        }
                        scriptArg = 7;
                    }
                    if (scriptArg >= argc && !createSink)
                    {
                        Tcl_AppendResult(interp, "Missing script for event handler creation", NULL);
                        return TCL_ERROR;
                    }
                    handler = new EventHandler(argv[4]);
                    if (createSink)
                        handler->setSink(eventType);
                    else if (eventType != Interactor::InvalidInput)
                        handler->setScript((char *)argv[scriptArg], eventType);
                    else
                        handler->setScript((char *)argv[scriptArg]);
                    adr = handler->getAddress();
                    adr->reparent(UserHandlerNeighborhood);
                    Tcl_SetResult(interp, (char *)adr->getAddress().c_str(), TCL_VOLATILE);
                    break;
                case DeleteCommand:
                    retval = findAddressIn(argv[4], UserHandlerNeighborhood, address);
                    if (retval != TCL_OK)
                        return retval;
                    if (address)
                    {
                        DYNAMIC_CAST(handler, EventHandler *, address);
                    }
                    if (!handler)
                    {
                        Tcl_AppendResult(interp, "Invalid Handler: ", argv[4]);
                        return TCL_ERROR;
                    }
                    delete handler;
                    break;
            }
            break;
        case CreateCommand:
            event = Interactor::newEventType(strdup(argv[3]));
            event->permanentEvent = GL_FALSE;
            Tcl_SetResult(interp, (char *)event->getAddress()->getAddress().c_str(), TCL_VOLATILE);
            break;
        case DeleteCommand:
            retval = findAddressIn(argv[3], EventNeighborhood, address);
            if (retval != TCL_OK)
                return retval;

            DYNAMIC_CAST(event, EventType *, address);
            if (!event)
            {
                Tcl_AppendResult(interp, "Invalid event: ", argv[3], NULL);
                return TCL_ERROR;
            }
            if (event->permanentEvent)
            {
                Tcl_AppendResult(interp, "System event ", argv[3], " cannot be deleted.", NULL);
                return TCL_ERROR;
            }
            delete event;
            break;
        case MediatorCommand:
            switch (subcommand)
            {
                case CreateCommand:
                    mediator = new InputMediator(strdup(argv[4]), GL_FALSE);
                    mediator->isPermanent = GL_FALSE;
                    Tcl_SetResult(interp, (char *)mediator->getAddress()->getAddress().c_str(), TCL_VOLATILE);
                    break;
                case DeleteCommand:
                    retval = findAddressIn(argv[4], EventMediatorNeighborhood, address);
                    if (retval != TCL_OK)
                        return retval;

                    DYNAMIC_CAST(mediator, InputMediator *, address);
                    if (!mediator)
                    {
                        Tcl_AppendResult(interp, "Invalid event mediator: ", argv[4], NULL);
                        return TCL_ERROR;
                    }
                    if (mediator->isPermanent)
                    {
                        Tcl_AppendResult(interp, "System event mediator ", argv[4], " cannot be deleted.", NULL);
                        return TCL_ERROR;
                    }
                    delete mediator;
                    break;
            }
        case TentacleCommand:
            switch (subcommand)
            {
                case CreateCommand:
                    tentacle = new InputTentacle(argv[4]);
                    Tcl_SetResult(interp, (char *)tentacle->getAddress()->getAddress().c_str(), TCL_VOLATILE);
                    break;
                case DeleteCommand:
                    retval = findAddressIn(argv[4], InteractorNeighborhood, address);
                    if (retval != TCL_OK)
                        return retval;

                    DYNAMIC_CAST(tentacle, InputTentacle *, address);
                    if (!tentacle)
                    {
                        Tcl_AppendResult(interp, "Invalid event tentacle: ", argv[4], NULL);
                        return TCL_ERROR;
                    }
                    delete tentacle;
                    break;
            }
        default:
            break;
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**
*   This function queries or sets the time buffering or real-time source
*   on the TimeControl
*
*   argv[2]     option (currently, only "buffer" or "rtSource")
*   argv[3]...  arguments
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::TimeCommand(int argc, const char *argv[])
{
    char retval[32];
    const char *val;
    int len;

    if (argc > 2)
    {
        if (!strcmp(argv[2], "buffer"))
        {
            if (argc > 3)
            {
                val = argv[3];
                if (val[0] == '+')
                {
                    scene3d->timeBufferBack(GL_TRUE);
                    if (val[1] == '-')
                        scene3d->timeBufferFront(GL_TRUE);
                    else
                        scene3d->timeBufferFront(GL_FALSE);
                    if (argc > 4)
                        val = argv[4];
                    else
                        val = NULL;
                }
                else if (val[0] == '-')
                {
                    scene3d->timeBufferFront(GL_TRUE);
                    if (val[1] == '+')
                        scene3d->timeBufferBack(GL_TRUE);
                    else
                        scene3d->timeBufferBack(GL_FALSE);
                    if (argc > 4)
                        val = argv[4];
                    else
                        val = NULL;
                }
                if (val)
                {
                    len = strlen(val);
                    if (val[len - 1] == '%')
                        scene3d->timeBufferFactor(atof(val) * .01);
                    else
                        scene3d->timeBufferAmount(atof(val));
                    return TCL_OK;
                }
            }
            if (scene3d->timeBufferFactor())
                sprintf(retval, "%g%%", scene3d->timeBufferFactor());
            else
                sprintf(retval, "%g", scene3d->timeBufferAmount());
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
        else if (!strcmp(argv[2], "rtSource"))
        {
            Addressable *address = NULL;
            Object *ob = NULL;
            RTSource *rts = NULL;
            TimeControl *tc = getScene3D()->timeControl;

            if (argc > 3)
            {
                int ret;

                if (!argv[3] || argv[3][0] == '\0' || (argv[3][0] == '{' && argv[3][1] == '}'))
                    tc->setRTsource(NULL);
                else
                {
                    //first, look for a regular object by the name
                    ret = findObject(argv[3], address, ob, GL_FALSE);
                    if (ret != TCL_OK)
                        return ret;
                    DYNAMIC_CAST(rts, RTSource *, ob);
                    if (!rts)
                    {
                        ret = findAddressIn(argv[3], ObjectNeighborhood, address);
                        if (ret != TCL_OK)
                            return ret;
                        if (address)
                            DYNAMIC_CAST(rts, RTSource *, address);
                    }
                    if (!rts)
                    {
                        Tcl_AppendResult(interp, argv[3], " is not a valid real-time source.  Discarding.", NULL);
                        return TCL_ERROR;
                    }
                    tc->setRTsource(rts);
                    tc->useRT();
                }
            }
            else
            {
                rts = tc->getRTsource();
                if (rts)
                {
                    DYNAMIC_CAST(address, Addressable *, rts);
                    if (!address)
                    {
                        DYNAMIC_CAST(ob, Object *, rts);
                        address = ob->getPosable();
                    }
                }
            }

            if (address)
                Tcl_SetResult(interp, (char *) address->getAddress()->getAddress().c_str(), TCL_VOLATILE);
            else
                Tcl_SetResult(interp, const_cast<char*>("{}"), TCL_VOLATILE);

            return TCL_OK;
        }
    }
    Tcl_AppendResult(interp, "Incorrect use of time subcommand.  Usage options:\n",
                        "gz time buffer [+ | - | +-] [<amount> | <percentage>%]\n",
                        "gz time rtSource [<realtimeSource>]\n",
                NULL);
    return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
/**
*   This function queries or sets the maxTransparencyLevel for the scene.
*
*   argv[2]     new transparency level (optional)
*
*   @return             TCL_OK / TCL_ERROR
*/
int gzCommandInterface::MaxTransparencyLevelCommand(int argc, const char *argv[])
{
    char retval[16];

    if (argc > 2)
        return scene3d->setMaxTransparencyLevel(atoi(argv[2]));

    sprintf(retval, "%d", scene3d->getMaxTransparencyLevel());
    Tcl_SetResult(interp, retval, TCL_VOLATILE);

    return TCL_OK;
}

/*--------------------------Object Bookkeeping-------------------------------*/
/*---------------------------------------------------------------------------*/
/**
*   This function sets the current Scene3D to the one passed.  Good for future
*   operations.
*/
void gzCommandInterface::setScene3D(Scene3D *s3d)
{
    scene3d = s3d;
}

/*---------------------------------------------------------------------------*/
/**
*   This function adds an object to the current Scene3D, and resets the
*   doneDrawingHighRes flag.
*
*   @param  object      Object to add to the scene.
*   @param  flags       Flags for futuer expansion (only to this window, or to whole scene, for instance).
*/
void gzCommandInterface::addObject(Object *object, int flags)
{
    scene3d->AddObject(object, interp);
}

/*---------------------------------------------------------------------------*/
/** This function looks for the passed posableName in the Posable hierarchy
*   (or root, if the name starts with a dot).  If found, returns the associated
*   pose and address information through the arguments and returns TCL_OK.
*   Otherwise TCL_ERROR is returned.  If something was at the address, but it
*   was not a posable, then the address return value is a reference to the
*   address record.  If the address is not found, an error is reported
*   regardless of the value of reportError.
*/
int gzCommandInterface::findPosable(string posableName, Addressable *&address,
                PosableStateBase *&pose, GLboolean reportError, DottedAddress *loc)
{
    int retval;

    pose = NULL;
    retval = findAddressIn(posableName, loc, address);
    if (retval != TCL_OK)
        return retval;

    if (address)
        DYNAMIC_CAST(pose, PosableStateBase*, address);

    if (!pose && reportError)
    {
        Tcl_AppendResult(interp, "Invalid posable: ", posableName.c_str(), NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This function looks for the passed name in the hierarchy of the passed
*   DottedAddress (or root, if the name starts with a dot).  If non-null, also
*   searches in neighborhood2.  If found, returns the associated address
*   information through the argument and returns TCL_OK.  Otherwise TCL_ERROR
*   is returned.  If the address is not found, an error is reported.
*/
int gzCommandInterface::findAddressIn(string name, DottedAddress *neighborhood,
                                    Addressable *&address,
                                    DottedAddress *neighborhood2)
{
    if (name.empty())
        return TCL_ERROR;

    if (name[0] == '.')
        address = rootAddress->findItem(name);
    else
    {
        address = neighborhood->findItem(name);
        if (neighborhood2 && !address)
            address = neighborhood2->findItem(name);
    }

    if (!address)
    {
        Tcl_AppendResult(interp, "Invalid address: ", name.c_str(), NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This function looks for the passed windowName in the Window hierarchy
*   (or root, if the name starts with a dot).  If found, returns the associated
*   Window3D * and address information through the arguments and returns TCL_OK.
*   Otherwise TCL_ERROR is returned.  If something was at the address, but it
*   was not a window, then the address return value is a reference to the
*   address record.  If the address is not found, an error is reported
*   regardless of the value of reportError.
*/
int gzCommandInterface::findWindow(string windowName, Addressable *&address,
                Window3D *&win, GLboolean reportError)
{
    PosableStateBase *p = NULL;
    PosableStateUser *u;
    ViewManager *v;
    int retval;

    win = NULL;
    retval = findAddressIn(windowName, WindowNeighborhood, address);
    if (retval != TCL_OK)
        return retval;

    if (address)
        DYNAMIC_CAST(p, PosableStateBase *, address);

    if (p)
    {
        u = p->getUser();
        DYNAMIC_CAST(v, ViewManager *, u);
        if (v)
            win = v->getParentWin();
    }
    if (!win && reportError)
    {
        Tcl_AppendResult(interp, "Invalid window: ", windowName.c_str(), NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This function looks for the passed objectName in the Object hierarchy
*   (or root, if the name starts with a dot).  If found, returns the associated
*   Object * and address information through the arguments and returns TCL_OK.
*   Otherwise TCL_ERROR is returned.  If something was at the address, but it
*   was not an Object, then the address return value is a reference to the
*   address record.  If the address is not found, an error is reported
*   regardless of the value of reportError.
*/
int gzCommandInterface::findObject(string objectName, Addressable *&address,
                Object *&ob, GLboolean reportError)
{
    PosableStateBase *p = NULL;
    PosableStateUser *u = NULL;
    int retval;

    ob = NULL;
    retval = findAddressIn(objectName, ObjectNeighborhood, address, PosableNeighborhood);
    if (retval != TCL_OK)
        return retval;

    if (address)
        DYNAMIC_CAST(p, PosableStateBase *, address);

    if (p)
    {
        u = p->getUser();
        DYNAMIC_CAST(ob, Object *, u);
    }
    if (!ob && reportError)
    {
        Tcl_AppendResult(interp, "Invalid object: ", objectName.c_str(), NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This function looks for the passed name first in the list of specific
*   events of the passed Interactor, and then in the hierarchy of Events
*   (or root, if the name starts with a dot).  If found, returns the
*   event value.  Otherwise Interactor::InvalidInput is returned.
*   If the event is not found, an error is reported.
*/
unsigned int gzCommandInterface::convertEvent(string name, Interactor *context)
{
    unsigned int eventType = Interactor::InvalidInput;
    int retval;
    Addressable *address;
    EventType *type;
    
    if (context)
        eventType = context->valueOfSpecificEventType(name);
    else
        eventType = mainWindow->valueOfSpecificEventType(name);

    if (eventType == Interactor::InvalidInput)
    {
        retval = findAddressIn(name, EventNeighborhood, address);
        if (retval != TCL_OK)
            return Interactor::InvalidInput;
        if (address)
        {
            DYNAMIC_CAST(type, EventType *, address);
            if (type)
                eventType = type->value | Interactor::EnumeratedAction;
        }
    }
    if (eventType == Interactor::InvalidInput)
        Tcl_AppendResult(interp, "Invalid event: ", name.c_str(), NULL);

    return eventType;
}

/*---------------------------------------------------------------------------*/
/** This function lists the children of the passed address in a Tcl-style
*   (space-delimited) list.  The result is set using Tcl_SetResult, rather
*   than being returned through C++.
*/
int gzCommandInterface::listChildrenOf(DottedAddress *parentAddress, GLboolean listFullName)
{
    stringstream returnString;
    DottedAddress *childAddress;

    for (int i = 0; (childAddress = parentAddress->getChild(i)) != NULL; i++)
    {
        if (i != 0)
            returnString << " ";
        string ca  = childAddress->getAddress();
        if (!listFullName)
            ca = ca.substr(parentAddress->getAddress().size() + 1); //+1 for the dot
        returnString << ca;
    }
    Tcl_SetResult(interp, (char *)returnString.str().c_str(), TCL_VOLATILE);
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This function returns the internal equivalent of the passed couplingString,
*   as defined in PosableStateLock.h.  If there a bad string is passed, -1 is
*   returned.  If reportError is set, an error is reported through Tcl.
*/
long gzCommandInterface::parseCouplingType(string couplingString,
                                        GLboolean reportError)
{
    long changeMask = PosableStateLock::LockFromOwner;

    for (string::size_type i = 0; i < couplingString.size(); i++)
    {
        switch (couplingString[i])
        {
            case 'T':
                changeMask |= PosableStateLock::LockTranslate;
                break;
            case 't':
                changeMask |= PosableStateLock::LockDeltaTranslate;
                break;
            case 'H':
                changeMask |= PosableStateLock::LockRotateY;
                break;
            case 'h':
                changeMask |= PosableStateLock::LockDeltaRotateY;
                break;
            case 'P':
                changeMask |= PosableStateLock::LockRotateX;
                break;
            case 'p':
                changeMask |= PosableStateLock::LockDeltaRotateX;
                break;
            case 'R':
                changeMask |= PosableStateLock::LockRotateZ;
                break;
            case 'r':
                changeMask |= PosableStateLock::LockDeltaRotateZ;
                break;
            case 'S':
                changeMask |= PosableStateLock::LockScale;
                break;
            case 's':
                changeMask |= PosableStateLock::LockDeltaScale;
                break;
            case 'l':
                changeMask |= PosableStateLock::LockLocalized;
                break;
            case 'd':
                changeMask |= PosableStateLock::LockDynamic;
                break;
            case '+':
                changeMask |= PosableStateLock::LockFromTarget;
                break;
            case '<':
                changeMask &= ~PosableStateLock::LockFromOwner;
                changeMask |= PosableStateLock::LockFromTarget;
                break;
            case ' ': case '\t': case '\n':
                break;
            default:
                Tcl_AppendResult(interp, "Bad coupling-type string.  Must consist of subset of the following:\n",
                                        " (t)T - (Relative-)couple translation\n",
                                        " (h)H - (Relative-)couple heading\n",
                                        " (p)P - (Relative-)couple pitch\n",
                                        " (r)R - (Relative-)couple roll\n",
                                        " (s)S - (Relative-)couple scale\n",
                                        "    l - Localized-relative coupling (apply scale, orientation to translation)\n",
                                        "    d - Dynamic coupling (one-way couplings allow target objects to \"rachet\")\n",
                                        "    + - Two-way coupling (changes in either will affect either)\n",
                                        "    < - Reverse coupling (changes in left-hand posable affect right-hand posable)\n",
                                        "For instance: \"THPR\" or \"T hpr\" or \"T\" or \"th\"\n",
                                        NULL);
                return -1;
        }
    }
    return changeMask;
}

/*---------------------------------------------------------------------------*/
/** This function returns a unique integer identifier (unless it is called
*   over 4 billion times).
*/
long gzCommandInterface::getUniqueCount()
{
    return uniqueCount++;
}

int gzCommandInterface::PostObjectMenu(int id)
{
    scene3d->PostObjectMenu(id);
    return TCL_OK;
}
